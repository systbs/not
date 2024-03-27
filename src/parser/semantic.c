/*
 * Creator: Yasser Sajjadi(Ys)
 * Date: 24/3/2024
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

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
#include "syntax.h"
#include "error.h"
#include "semantic.h"

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
semantic_postfix(program_t *program, node_t *node, list_t *response);

static int32_t
semantic_expression(program_t *program, node_t *node, list_t *response);

static int32_t
semantic_body(program_t *program, node_t *node);


/*
 * @Note: n1 is subset of n2
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
                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r2 = semantic_postfix(program, heritage2->type, response2);
                    if (r2 == -1)
                    {
                        return -1;
                    }
                    else
                    if (r2 == 0)
                    {
                        semantic_error(program, heritage2->type, "reference not found");
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
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    
                                    int32_t r1 = semantic_postfix(program, generic1->type, response1);
                                    if (r1 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r1 == 0)
                                    {
                                        semantic_error(program, generic1->type, "reference not found");
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
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            
                                            int32_t r2 = semantic_postfix(program, generic2->type, response2);
                                            if (r2 == -1)
                                            {
                                                return -1;
                                            }
                                            else
                                            if (r2 == 0)
                                            {
                                                semantic_error(program, generic2->type, "reference not found");
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
                                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }

                                        int32_t r1 = semantic_postfix(program, generic1->type, response1);
                                        if (r1 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r1 == 0)
                                        {
                                            semantic_error(program, generic1->type, "reference not found");
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
                                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                
                                                int32_t r2 = semantic_postfix(program, generic2->type, response2);
                                                if (r2 == -1)
                                                {
                                                    return -1;
                                                }
                                                else
                                                if (r2 == 0)
                                                {
                                                    semantic_error(program, generic2->type, "reference not found");
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
                                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }

                                        int32_t r1 = semantic_postfix(program, generic1->type, response1);
                                        if (r1 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r1 == 0)
                                        {
                                            semantic_error(program, generic1->type, "reference not found");
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
                                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                
                                                int32_t r2 = semantic_postfix(program, generic2->type, response2);
                                                if (r2 == -1)
                                                {
                                                    return -1;
                                                }
                                                else
                                                if (r2 == 0)
                                                {
                                                    semantic_error(program, generic2->type, "reference not found");
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
                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }
                            
                            int32_t r1 = semantic_postfix(program, parameter1->type, response1);
                            if (r1 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r1 == 0)
                            {
                                semantic_error(program, parameter1->type, "reference not found");
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
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    
                                    int32_t r2 = semantic_postfix(program, parameter2->type, response2);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, parameter2->type, "reference not found");
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
                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                
                                int32_t r1 = semantic_postfix(program, parameter1->type, response1);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 0)
                                {
                                    semantic_error(program, parameter1->type, "reference not found");
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
                                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }
                                        
                                        int32_t r2 = semantic_postfix(program, parameter2->type, response2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, parameter2->type, "reference not found");
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
                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                
                                int32_t r1 = semantic_postfix(program, parameter1->type, response1);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 0)
                                {
                                    semantic_error(program, parameter1->type, "reference not found");
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
                                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }
                                        
                                        int32_t r2 = semantic_postfix(program, parameter2->type, response2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, parameter2->type, "reference not found");
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
            uint64_t cnt1 = 0, cnt2 = 0;
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
                    node_block_t *bas2 = (node_block_t *)nas2->value;
                    ilist_t *a2;
                    for (a2 = bas2->list->begin;a2 != bas2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_ARGUMENT)
                        {
                            cnt2 += 1;
                            if (cnt2 < cnt1)
                            {
                                continue;
                            }
                            node_argument_t *argument2 = (node_argument_t *)item2->value;
                            
                            if (parameter1->type != NULL)
                            {
                                list_t *response1 = list_create();
                                if (response1 == NULL)
                                {
                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                
                                int32_t r1 = semantic_postfix(program, parameter1->type, response1);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 0)
                                {
                                    semantic_error(program, parameter1->type, "reference not found");
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
                                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }
                                        
                                        int32_t r2 = semantic_postfix(program, argument2->value, response2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, argument2->value, "reference not found");
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
                                                    goto region_by_name_check;
                                                    
                                                }
                                            }
                                        }
                                        list_destroy(response2);
                                    }
                                
                                    list_destroy(response1);
                                }
                            }
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
            node_block_t *bas2 = (node_block_t *)nas2->value;
            ilist_t *a2;
            for (a2 = bas2->list->begin;a2 != bas2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->kind == NODE_KIND_PARAMETER)
                {
                    cnt2 += 1;
                    if (cnt1 > cnt2)
                    {
                        goto region_by_name_check;
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
                    node_block_t *bas2 = (node_block_t *)nas2->value;
                    ilist_t *a2;
                    for (a2 = bas2->list->begin;a2 != bas2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_ARGUMENT)
                        {
                            node_argument_t *argument2 = (node_argument_t *)item2->value;
                            if (semantic_idcmp(parameter1->key, argument2->key) == 1)
                            {
                                found = 1;
                                if (parameter1->type != NULL)
                                {
                                    if (argument2->value != NULL)
                                    {
                                        list_t *response1 = list_create();
                                        if (response1 == NULL)
                                        {
                                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }

                                        int32_t r1 = semantic_postfix(program, parameter1->type, response1);
                                        if (r1 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r1 == 0)
                                        {
                                            semantic_error(program, parameter1->type, "reference not found");
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
                                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                
                                                int32_t r2 = semantic_postfix(program, argument2->value, response2);
                                                if (r2 == -1)
                                                {
                                                    return -1;
                                                }
                                                else
                                                if (r2 == 0)
                                                {
                                                    semantic_error(program, argument2->value, "reference not found");
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
                                                            return 0;
                                                        }
                                                    }
                                                }
                                                list_destroy(response2);
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
                        if (parameter1->value == NULL)
                        {
                            return 0;
                        }
                    }
                }
            }

            bas2 = (node_block_t *)nas2->value;
            for (a2 = bas2->list->begin;a2 != bas2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->kind == NODE_KIND_ARGUMENT)
                {
                    node_argument_t *argument2 = (node_argument_t *)item2->value;

                    int32_t found = 0;
                    for (a1 = bps1->list->begin;a1 != bps1->list->end;a1 = a1->next)
                    {
                        node_t *item1 = (node_t *)a1->value;
                        if (item1->kind == NODE_KIND_PARAMETER)
                        {
                            node_parameter_t *parameter1 = (node_parameter_t *)item1->value;

                            if (semantic_idcmp(parameter1->key, argument2->key) == 1)
                            {
                                found = 1;
                                if (parameter1->type != NULL)
                                {
                                    if (argument2->value != NULL)
                                    {
                                        list_t *response1 = list_create();
                                        if (response1 == NULL)
                                        {
                                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }

                                        int32_t r1 = semantic_postfix(program, parameter1->type, response1);
                                        if (r1 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r1 == 0)
                                        {
                                            semantic_error(program, parameter1->type, "reference not found");
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
                                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                
                                                int32_t r2 = semantic_postfix(program, argument2->value, response2);
                                                if (r2 == -1)
                                                {
                                                    return -1;
                                                }
                                                else
                                                if (r2 == 0)
                                                {
                                                    semantic_error(program, argument2->value, "reference not found");
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
                                                            return 0;
                                                        }
                                                    }
                                                }
                                                list_destroy(response2);
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
                        return 0;
                    }
                }
            }

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
                    node_block_t *bfs2 = (node_block_t *)nfs2->value;
                    ilist_t *a2;
                    for (a2 = bfs2->list->begin;a2 != bfs2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_FIELD)
                        {
                            cnt2 += 1;
                            if (cnt2 < cnt1)
                            {
                                continue;
                            }
                            node_field_t *field2 = (node_field_t *)item2->value;
                            
                            if (generic1->type != NULL)
                            {
                                list_t *response1 = list_create();
                                if (response1 == NULL)
                                {
                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                
                                int32_t r1 = semantic_postfix(program, generic1->type, response1);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 0)
                                {
                                    semantic_error(program, generic1->type, "reference not found");
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
                                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }
                                        
                                        int32_t r2 = semantic_postfix(program, field2->key, response2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, field2->key, "reference not found");
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
                            }
                            break;
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
            node_block_t *bfs2 = (node_block_t *)nfs2->value;
            ilist_t *a2;
            for (a2 = bfs2->list->begin;a2 != bfs2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->kind == NODE_KIND_FIELD)
                {
                    cnt2 += 1;
                    if (cnt2 > cnt1)
                    {
                        goto region_by_name_check;
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
                    node_block_t *bps2 = (node_block_t *)nfs2->value;
                    ilist_t *a2;
                    for (a2 = bps2->list->begin;a2 != bps2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_FIELD)
                        {
                            node_field_t *field2 = (node_field_t *)item2->value;
                            if (semantic_idcmp(generic1->key, field2->key) == 1)
                            {
                                found = 1;
                                if (generic1->type != NULL)
                                {
                                    if (field2->value != NULL)
                                    {
                                        list_t *response1 = list_create();
                                        if (response1 == NULL)
                                        {
                                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }

                                        int32_t r1 = semantic_postfix(program, generic1->type, response1);
                                        if (r1 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r1 == 0)
                                        {
                                            semantic_error(program, generic1->type, "reference not found");
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
                                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                
                                                int32_t r2 = semantic_postfix(program, field2->value, response2);
                                                if (r2 == -1)
                                                {
                                                    return -1;
                                                }
                                                else
                                                if (r2 == 0)
                                                {
                                                    semantic_error(program, field2->value, "reference not found");
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

            bfs2 = (node_block_t *)nfs2->value;
            for (a2 = bfs2->list->begin;a2 != bfs2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->kind == NODE_KIND_FIELD)
                {
                    node_field_t *field2 = (node_field_t *)item2->value;

                    int32_t found = 0;
                    for (a1 = bps1->list->begin;a1 != bps1->list->end;a1 = a1->next)
                    {
                        node_t *item1 = (node_t *)a1->value;
                        if (item1->kind == NODE_KIND_GENERIC)
                        {
                            node_generic_t *generic1 = (node_generic_t *)item1->value;

                            if (semantic_idcmp(generic1->key, field2->key) == 1)
                            {
                                found = 1;
                                if (generic1->type != NULL)
                                {
                                    if (field2->value != NULL)
                                    {
                                        list_t *response1 = list_create();
                                        if (response1 == NULL)
                                        {
                                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }

                                        int32_t r1 = semantic_postfix(program, generic1->type, response1);
                                        if (r1 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r1 == 0)
                                        {
                                            semantic_error(program, generic1->type, "reference not found");
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
                                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                
                                                int32_t r2 = semantic_postfix(program, field2->value, response2);
                                                if (r2 == -1)
                                                {
                                                    return -1;
                                                }
                                                else
                                                if (r2 == 0)
                                                {
                                                    semantic_error(program, field2->value, "reference not found");
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
                        return 0;
                    }
                }
            }

            return 1;
        }
    }
    return 1;
}


static int32_t
semantic_select(program_t *program, node_t *root, node_t *scope, node_t *name, list_t *response, int32_t follow)
{
    if (root->kind == NODE_KIND_TRY)
    {
        node_try_t *try1 = (node_try_t *)root->value;
        if (try1->generics != NULL)
        {
            node_t *node1 = try1->generics;
            node_block_t *block1 = (node_block_t *)node1->value;
            ilist_t *a1;
            for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_GENERIC)
                {
                    node_generic_t *generic1 = (node_generic_t *)item1->value;
                    if (generic1->key != NULL)
                    {
                        if (semantic_idcmp(generic1->key, name) == 1)
                        {
                            ilist_t *r1 = list_rpush(response, item1);
                            if (r1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                }
            }
        }
    }
    else
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
                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }
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
                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }
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
                            if (item3->kind == NODE_KIND_PROPERTY)
                            {
                                node_property_t *property1 = (node_property_t *)item3->value;
                                if (semantic_idcmp(property1->key, name) == 1)
                                {
                                    ilist_t *r1 = list_rpush(response, item3);
                                    if (r1 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
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
                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }
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
                            if (item3->kind == NODE_KIND_PROPERTY)
                            {
                                node_property_t *property1 = (node_property_t *)item3->value;
                                if (semantic_idcmp(property1->key, name) == 1)
                                {
                                    ilist_t *r1 = list_rpush(response, item3);
                                    if (r1 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
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
                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }
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
                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }
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
                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }
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
                        if (item2->kind == NODE_KIND_PROPERTY)
                        {
                            node_property_t *property2 = (node_property_t *)item2->value;
                            if (semantic_idcmp(property2->key, name) == 1)
                            {
                                ilist_t *r1 = list_rpush(response, item2);
                                if (r1 == NULL)
                                {
                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
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
    if (root->kind == NODE_KIND_CLASS)
    {
        node_class_t *class1 = (node_class_t *)root->value;
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
                    if (scope != NULL)
                    {
                        item1->flag |= NODE_FLAG_DERIVE;

                        if ((class2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                        {
                            semantic_error(program, item1, "private access");
                            return -1;
                        }

                        if (follow != 1)
                        {
                            if ((class2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                            {
                                semantic_error(program, item1, "protect access");
                                return -1;
                            }
                        }
                    }
                    else
                    {
                        item1->flag &= ~NODE_FLAG_DERIVE;
                    }
                    ilist_t *r1 = list_rpush(response, item1);
                    if (r1 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                }
            }
            else
            if (item1->kind == NODE_KIND_ENUM)
            {
                node_enum_t *enum1 = (node_enum_t *)item1->value;
                if (semantic_idcmp(enum1->key, name) == 1)
                {
                    if (scope != NULL)
                    {
                        item1->flag |= NODE_FLAG_DERIVE;

                        if ((enum1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                        {
                            semantic_error(program, item1, "private access");
                            return -1;
                        }

                        if (follow != 1)
                        {
                            if ((enum1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                            {
                                semantic_error(program, item1, "protect access");
                                return -1;
                            }
                        }
                    }
                    else
                    {
                        item1->flag &= ~NODE_FLAG_DERIVE;
                    }
                    ilist_t *r1 = list_rpush(response, item1);
                    if (r1 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                }
            }
            else
            if (item1->kind == NODE_KIND_FUN)
            {
                node_fun_t *fun1 = (node_fun_t *)item1->value;
                if (semantic_idcmp(fun1->key, name) == 1)
                {
                    if (scope != NULL)
                    {
                        item1->flag |= NODE_FLAG_DERIVE;

                        if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                        {
                            semantic_error(program, item1, "private access");
                            return -1;
                        }

                        if (follow != 1)
                        {
                            if ((fun1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                            {
                                semantic_error(program, item1, "protect access");
                                return -1;
                            }
                        }
                    }
                    else
                    {
                        item1->flag &= ~NODE_FLAG_DERIVE;
                    }
                    ilist_t *r1 = list_rpush(response, item1);
                    if (r1 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                }
            }
            else
            if (item1->kind == NODE_KIND_PROPERTY)
            {
                node_fun_t *property1 = (node_fun_t *)item1->value;
                if (semantic_idcmp(property1->key, name) == 1)
                {
                    if (scope != NULL)
                    {
                        item1->flag |= NODE_FLAG_DERIVE;

                        if ((property1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                        {
                            semantic_error(program, item1, "private access");
                            return -1;
                        }

                        if (follow != 1)
                        {
                            if ((property1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                            {
                                semantic_error(program, item1, "protect access");
                                return -1;
                            }
                        }
                    }
                    else
                    {
                        item1->flag &= ~NODE_FLAG_DERIVE;
                    }
                    ilist_t *r1 = list_rpush(response, item1);
                    if (r1 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                }
            }
        }

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
                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }
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
                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    if (name->id == heritage3->type->id)
                    {
                        break;
                    }

                    if (heritage3->value_update == NULL)
                    {
                        list_t *response2 = list_create();
                        if (response2 == NULL)
                        {
                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }
                        
                        int32_t r1 = semantic_postfix(program, heritage3->type, response2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 0)
                        {
                            semantic_error(program, heritage3->type, "reference not found");
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            ilist_t *a3;
                            for (a3 = response2->begin;a3 != response2->end;a3 = a3->next)
                            {
                                node_t *item3 = (node_t *)a3->value;
                                if (item3->kind == NODE_KIND_CLASS)
                                {
                                    heritage3->value_update = item3;
                                    int32_t r2 = semantic_select(program, item3, NULL, name, response, 1);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                }
                                else
                                {
                                    semantic_error(program, item3, "non-class as heritage, for (%lld:%lld)",
                                        heritage3->type->position.line, heritage3->type->position.column);
                                    return -1;
                                }
                                break;
                            }
                        }
                        list_destroy(response2);
                    }
                    else
                    {
                        int32_t r2 = semantic_select(program, heritage3->value_update, NULL, name, response, 1);
                        if (r2 == -1)
                        {
                            return -1;
                        }
                    }
                }
            }
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
            if (item1->kind == NODE_KIND_IMPORT)
            {
                node_import_t *import1 = (node_import_t *)item1->value;
                node_t *node2 = import1->packages;
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
                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                }
            }
            else
            if (item1->kind == NODE_KIND_CLASS)
            {
                node_class_t *class2 = (node_class_t *)item1->value;
                if (semantic_idcmp(class2->key, name) == 1)
                {
                    if (scope != NULL)
                    {
                        item1->flag |= NODE_FLAG_DERIVE;
                    }
                    else
                    {
                        item1->flag &= ~NODE_FLAG_DERIVE;
                    }

                    ilist_t *r1 = list_rpush(response, item1);
                    if (r1 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                }
            }
            else
            if (item1->kind == NODE_KIND_ENUM)
            {
                node_enum_t *enum1 = (node_enum_t *)item1->value;
                if (semantic_idcmp(enum1->key, name) == 1)
                {
                    if (scope != NULL)
                    {
                        item1->flag |= NODE_FLAG_DERIVE;
                    }
                    else
                    {
                        item1->flag &= ~NODE_FLAG_DERIVE;
                    }

                    ilist_t *r1 = list_rpush(response, item1);
                    if (r1 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                }
            }
        }
    }
    

    if (root->parent != NULL && (follow != 1))
    {
        if ((root->kind == NODE_KIND_CLASS) && (scope == NULL))
        {
            return semantic_select(program, root->parent, root, name, response, follow);
        }
        else
        {
            return semantic_select(program, root->parent, scope, name, response, follow);
        }
    }

    if (list_count(response) > 0)
    {
        return 1;
    }

    return 0;
}

static int32_t
semantic_id(program_t *program, node_t *node, list_t *response)
{
    return semantic_select(program, node->parent, NULL, node, response, 0);
}

static int32_t
semantic_array(program_t *program, node_t *node, list_t *response)
{
    ilist_t *r1 = list_rpush(response, node);
    if (r1 == NULL)
    {
        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }
    return 1;
}

static int32_t
semantic_object(program_t *program, node_t *node, list_t *response)
{
    ilist_t *r1 = list_rpush(response, node);
    if (r1 == NULL)
    {
        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }
    return 1;
}

static int32_t
semantic_primary(program_t *program, node_t *node, list_t *response)
{
    if (node->kind == NODE_KIND_ID)
    {
        return semantic_id(program, node, response);
    }
    else
    if (node->kind == NODE_KIND_ARRAY)
    {
        return semantic_array(program, node, response);
    }
    else
    if (node->kind == NODE_KIND_OBJECT)
    {
        return semantic_object(program, node, response);
    }
    else
    {
        semantic_error(program, node, "not a type");
        return -1;
    }
}

static int32_t
semantic_pseudonym(program_t *program, node_t *node, list_t *response)
{
    node_carrier_t *carrier = (node_carrier_t *)node->value;
    
    list_t *response1 = list_create();
    if (response1 == NULL)
    {
        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    int32_t r1 = semantic_postfix(program, carrier->base, response1);
    if (r1 == -1)
    {
        return -1;
    }
    else
    if (r1 == 0)
    {
        semantic_error(program, carrier->base, "reference not found");
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
                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
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
                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                }
            }
            else
            {
                semantic_error(program, item1, "not a class, in (%lld:%lld)",
                    node->position.line, node->position.column);
                return -1;
            }
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
semantic_attribute(program_t *program, node_t *node, list_t *response)
{
    node_binary_t *basic = (node_binary_t *)node->value;
    
    list_t *response1 = list_create();
    if (response1 == NULL)
    {
        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    int32_t r1 = semantic_postfix(program, basic->left, response1);
    if (r1 == -1)
    {
        return -1;
    }
    else
    if (r1 == 0)
    {
        semantic_error(program, basic->left, "reference not found");
        return -1;
    }
    else
    if (r1 == 1)
    {
        ilist_t *a1;
        for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
        {
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
                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }
                            list_destroy(response1);
                            return 1;
                        }
                    }
                }
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
                                    semantic_error(program, item2, "private access");
                                    return -1;
                                }

                                if ((class2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                {
                                    semantic_error(program, item1, "protect access");
                                    return -1;
                                }

                                item2->flag |= NODE_FLAG_DERIVE;

                                ilist_t *r2 = list_rpush(response, item2);
                                if (r2 == NULL)
                                {
                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                list_destroy(response1);
                                return 1;
                            }
                            else
                            {
                                item2->flag &= ~NODE_FLAG_DERIVE;

                                ilist_t *r2 = list_rpush(response, item2);
                                if (r2 == NULL)
                                {
                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                list_destroy(response1);
                                return 1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_ENUM)
                    {
                        node_enum_t *enum1 = (node_enum_t *)item2->value;
                        if (semantic_idcmp(basic->right, enum1->key) == 1)
                        {
                            if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                            {
                                if ((enum1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                {
                                    semantic_error(program, item2, "private access");
                                    return -1;
                                }

                                if ((enum1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                {
                                    semantic_error(program, item1, "protect access");
                                    return -1;
                                }

                                item2->flag |= NODE_FLAG_DERIVE;

                                ilist_t *r2 = list_rpush(response, item2);
                                if (r2 == NULL)
                                {
                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                list_destroy(response1);
                                return 1;
                            }
                            else
                            {
                                item2->flag &= ~NODE_FLAG_DERIVE;
                                
                                ilist_t *r2 = list_rpush(response, item2);
                                if (r2 == NULL)
                                {
                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                list_destroy(response1);
                                return 1;
                            }
                        }
                    }
                }

                if (class1->heritages != NULL)
                {
                    node_t *root = class1->heritages;
                    list_t *repository1 = list_create();
                    if (repository1 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    while (root != NULL)
                    {
                        node_block_t *block3 = (node_block_t *)root->value;

                        ilist_t *a3;
                        for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                        {
                            node_t *item3 = (node_t *)a3->value;
                            
                            if (item3->kind == NODE_KIND_HERITAGE)
                            {
                                node_heritage_t *heritage4 = (node_heritage_t *)item3->value;
                             
                                if (heritage4->value_update == NULL)
                                {
                                    list_t *response4 = list_create();
                                    if (response4 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                
                                    int32_t r4 = semantic_postfix(program, heritage4->type, response4);
                                    if (r4 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r4 == 0)
                                    {
                                        semantic_error(program, heritage4->type, "reference not found");
                                        return -1;
                                    }
                                    else
                                    if (r4 == 1)
                                    {
                                        ilist_t *a5;
                                        for (a5 = response4->begin;a5 != response4->end;a5 = a5->next)
                                        {
                                            node_t *item5 = (node_t *)a5->value;

                                            heritage4->value_update = item5;

                                            if (item5->kind == NODE_KIND_CLASS)
                                            {
                                                node_class_t *class5 = (node_class_t *)item5->value;
                                                node_t *node5 = class5->block;
                                                node_block_t *block5 = (node_block_t *)node5->value;

                                                ilist_t *a6;
                                                for (a6 = block5->list->begin;a6 != block5->list->end;a6 = a6->next)
                                                {
                                                    node_t *item6 = (node_t *)a6->value;
                                                    if (item6->kind == NODE_KIND_CLASS)
                                                    {
                                                        node_class_t *class6 = (node_class_t *)item6->value;
                                                        if (semantic_idcmp(class6->key, basic->right) == 1)
                                                        {
                                                            if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                            {
                                                                if ((class6->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                {
                                                                    semantic_error(program, item6, "private access");
                                                                    return -1;
                                                                }

                                                                if ((class6->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                {
                                                                    semantic_error(program, item6, "protect access");
                                                                    return -1;
                                                                }

                                                                item6->flag |= NODE_FLAG_DERIVE;

                                                                ilist_t *r6 = list_rpush(response, item6);
                                                                if (r6 == NULL)
                                                                {
                                                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                                    return -1;
                                                                }
                                                                list_destroy(response4);
                                                                return 1;
                                                            }
                                                            else
                                                            {
                                                                item6->flag &= ~NODE_FLAG_DERIVE;

                                                                ilist_t *r6 = list_rpush(response, item6);
                                                                if (r6 == NULL)
                                                                {
                                                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                                    return -1;
                                                                }
                                                                list_destroy(response4);
                                                                return 1;
                                                            }
                                                        }
                                                    }
                                                    else
                                                    if (item6->kind == NODE_KIND_ENUM)
                                                    {
                                                        node_enum_t *enum1 = (node_enum_t *)item6->value;
                                                        if (semantic_idcmp(enum1->key, basic->right) == 1)
                                                        {
                                                            if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                            {
                                                                if ((enum1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                {
                                                                    semantic_error(program, item6, "private access");
                                                                    return -1;
                                                                }

                                                                if ((enum1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                {
                                                                    semantic_error(program, item6, "protect access");
                                                                    return -1;
                                                                }

                                                                item6->flag |= NODE_FLAG_DERIVE;

                                                                ilist_t *r6 = list_rpush(response, item6);
                                                                if (r6 == NULL)
                                                                {
                                                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                                    return -1;
                                                                }
                                                                list_destroy(response4);
                                                                return 1;
                                                            }
                                                            else
                                                            {
                                                                item6->flag &= ~NODE_FLAG_DERIVE;

                                                                ilist_t *r6 = list_rpush(response, item6);
                                                                if (r6 == NULL)
                                                                {
                                                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                                    return -1;
                                                                }
                                                                list_destroy(response4);
                                                                return 1;
                                                            }
                                                        }
                                                    }
                                                }

                                                ilist_t *r3 = list_rpush(repository1, class5->heritages);
                                                if (r3 == NULL)
                                                {
                                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                            }
                                            else
                                            {
                                                semantic_error(program, item5, "non-class as heritage, for (%lld:%lld)",
                                                    heritage4->type->position.line, heritage4->type->position.column);
                                                return -1;
                                            }
                                        }
                                    }

                                    list_destroy(response4);
                                }
                                else
                                {
                                    node_t *item5 = heritage4->value_update;
                                    node_class_t *class5 = (node_class_t *)item5->value;
                                    node_t *node5 = class5->block;
                                    node_block_t *block5 = (node_block_t *)node5->value;

                                    ilist_t *a6;
                                    for (a6 = block5->list->begin;a6 != block5->list->end;a6 = a6->next)
                                    {
                                        node_t *item6 = (node_t *)a6->value;
                                        if (item6->kind == NODE_KIND_CLASS)
                                        {
                                            node_class_t *class6 = (node_class_t *)item6->value;
                                            if (semantic_idcmp(class6->key, basic->right) == 1)
                                            {
                                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                {
                                                    if ((class6->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                    {
                                                        semantic_error(program, item6, "private access");
                                                        return -1;
                                                    }

                                                    if ((class6->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                    {
                                                        semantic_error(program, item6, "protect access");
                                                        return -1;
                                                    }

                                                    item6->flag |= NODE_FLAG_DERIVE;

                                                    ilist_t *r6 = list_rpush(response, item6);
                                                    if (r6 == NULL)
                                                    {
                                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                    return 1;
                                                }
                                                else
                                                {
                                                    item6->flag &= ~NODE_FLAG_DERIVE;

                                                    ilist_t *r6 = list_rpush(response, item6);
                                                    if (r6 == NULL)
                                                    {
                                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                    return 1;
                                                }
                                            }
                                        }
                                        else
                                        if (item6->kind == NODE_KIND_ENUM)
                                        {
                                            node_enum_t *enum1 = (node_enum_t *)item6->value;
                                            if (semantic_idcmp(enum1->key, basic->right) == 1)
                                            {
                                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                {
                                                    if ((enum1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                    {
                                                        semantic_error(program, item6, "private access");
                                                        return -1;
                                                    }

                                                    if ((enum1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                    {
                                                        semantic_error(program, item6, "protect access");
                                                        return -1;
                                                    }

                                                    item6->flag |= NODE_FLAG_DERIVE;

                                                    ilist_t *r6 = list_rpush(response, item6);
                                                    if (r6 == NULL)
                                                    {
                                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                    return 1;
                                                }
                                                else
                                                {
                                                    item6->flag &= ~NODE_FLAG_DERIVE;

                                                    ilist_t *r6 = list_rpush(response, item6);
                                                    if (r6 == NULL)
                                                    {
                                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                    return 1;
                                                }
                                            }
                                        }
                                    }

                                    ilist_t *r3 = list_rpush(repository1, class5->heritages);
                                    if (r3 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                }
                            }
                        }
                    
                        ilist_t *r3 = list_rpop(repository1);
                        if (r3 != NULL)
                        {
                            root = (node_t *)r3->value;
                            continue;
                        }
                        else
                        {
                            root = NULL;
                            break;
                        }
                    }

                    list_destroy(repository1);
                }
            }
            else
            if (item1->kind == NODE_KIND_VAR)
            {
                node_var_t *var1 = (node_var_t *)item1;
                if (var1->value_update == NULL)
                {
                    semantic_error(program, item1, "not initialized");
                    return -1;
                }

                node_t *item2 = var1->value_update;
                if (item2->kind == NODE_KIND_CLASS)
                {
                    node_class_t *class1 = (node_class_t *)item2->value;
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
                                        semantic_error(program, item2, "private access");
                                        return -1;
                                    }

                                    if ((class2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                    {
                                        semantic_error(program, item1, "protect access");
                                        return -1;
                                    }

                                    item2->flag |= NODE_FLAG_DERIVE;

                                    ilist_t *r2 = list_rpush(response, item2);
                                    if (r2 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    list_destroy(response1);
                                    return 1;
                                }
                                else
                                {
                                    item2->flag &= ~NODE_FLAG_DERIVE;

                                    ilist_t *r2 = list_rpush(response, item2);
                                    if (r2 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    list_destroy(response1);
                                    return 1;
                                }
                            }
                        }
                        else
                        if (item2->kind == NODE_KIND_ENUM)
                        {
                            node_enum_t *enum1 = (node_enum_t *)item2->value;
                            if (semantic_idcmp(basic->right, enum1->key) == 1)
                            {
                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                {
                                    if ((enum1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                    {
                                        semantic_error(program, item2, "private access");
                                        return -1;
                                    }

                                    if ((enum1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                    {
                                        semantic_error(program, item1, "protect access");
                                        return -1;
                                    }

                                    item2->flag |= NODE_FLAG_DERIVE;

                                    ilist_t *r2 = list_rpush(response, item2);
                                    if (r2 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    list_destroy(response1);
                                    return 1;
                                }
                                else
                                {
                                    item2->flag &= ~NODE_FLAG_DERIVE;
                                    
                                    ilist_t *r2 = list_rpush(response, item2);
                                    if (r2 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    list_destroy(response1);
                                    return 1;
                                }
                            }
                        }
                    }

                    if (class1->heritages != NULL)
                    {
                        node_t *root = class1->heritages;
                        list_t *repository1 = list_create();
                        if (repository1 == NULL)
                        {
                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }

                        while (root != NULL)
                        {
                            node_block_t *block3 = (node_block_t *)root->value;

                            ilist_t *a3;
                            for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                            {
                                node_t *item3 = (node_t *)a3->value;
                                
                                if (item3->kind == NODE_KIND_HERITAGE)
                                {
                                    node_heritage_t *heritage4 = (node_heritage_t *)item3->value;
                                    
                                    if (heritage4->value_update == NULL)
                                    {
                                        list_t *response4 = list_create();
                                        if (response4 == NULL)
                                        {
                                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }
                                        
                                        int32_t r4 = semantic_postfix(program, heritage4->type, response4);
                                        if (r4 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r4 == 0)
                                        {
                                            semantic_error(program, heritage4->type, "reference not found");
                                            return -1;
                                        }
                                        else
                                        if (r4 == 1)
                                        {
                                            ilist_t *a5;
                                            for (a5 = response4->begin;a5 != response4->end;a5 = a5->next)
                                            {
                                                node_t *item5 = (node_t *)a5->value;
                                                
                                                heritage4->value_update = item5;

                                                if (item5->kind == NODE_KIND_CLASS)
                                                {
                                                    node_class_t *class5 = (node_class_t *)item5->value;
                                                    node_t *node5 = class5->block;
                                                    node_block_t *block5 = (node_block_t *)node5->value;

                                                    ilist_t *a6;
                                                    for (a6 = block5->list->begin;a6 != block5->list->end;a6 = a6->next)
                                                    {
                                                        node_t *item6 = (node_t *)a6->value;
                                                        if (item6->kind == NODE_KIND_CLASS)
                                                        {
                                                            node_class_t *class6 = (node_class_t *)item6->value;
                                                            if (semantic_idcmp(class6->key, basic->right) == 1)
                                                            {
                                                                if ((item2->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                {
                                                                    if ((class6->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                    {
                                                                        semantic_error(program, item6, "private access");
                                                                        return -1;
                                                                    }

                                                                    if ((class6->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                    {
                                                                        semantic_error(program, item6, "protect access");
                                                                        return -1;
                                                                    }

                                                                    item6->flag |= NODE_FLAG_DERIVE;

                                                                    ilist_t *r6 = list_rpush(response, item6);
                                                                    if (r6 == NULL)
                                                                    {
                                                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                                        return -1;
                                                                    }
                                                                    list_destroy(response4);
                                                                    return 1;
                                                                }
                                                                else
                                                                {
                                                                    item6->flag &= ~NODE_FLAG_DERIVE;

                                                                    ilist_t *r6 = list_rpush(response, item6);
                                                                    if (r6 == NULL)
                                                                    {
                                                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                                        return -1;
                                                                    }
                                                                    list_destroy(response4);
                                                                    return 1;
                                                                }
                                                            }
                                                        }
                                                        else
                                                        if (item6->kind == NODE_KIND_ENUM)
                                                        {
                                                            node_enum_t *enum1 = (node_enum_t *)item6->value;
                                                            if (semantic_idcmp(enum1->key, basic->right) == 1)
                                                            {
                                                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                {
                                                                    if ((enum1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                    {
                                                                        semantic_error(program, item6, "private access");
                                                                        return -1;
                                                                    }

                                                                    if ((enum1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                    {
                                                                        semantic_error(program, item6, "protect access");
                                                                        return -1;
                                                                    }

                                                                    item6->flag |= NODE_FLAG_DERIVE;

                                                                    ilist_t *r6 = list_rpush(response, item6);
                                                                    if (r6 == NULL)
                                                                    {
                                                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                                        return -1;
                                                                    }
                                                                    list_destroy(response4);
                                                                    return 1;
                                                                }
                                                                else
                                                                {
                                                                    item6->flag &= ~NODE_FLAG_DERIVE;

                                                                    ilist_t *r6 = list_rpush(response, item6);
                                                                    if (r6 == NULL)
                                                                    {
                                                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                                        return -1;
                                                                    }
                                                                    list_destroy(response4);
                                                                    return 1;
                                                                }
                                                            }
                                                        }
                                                    }

                                                    ilist_t *r3 = list_rpush(repository1, class5->heritages);
                                                    if (r3 == NULL)
                                                    {
                                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                }
                                                else
                                                {
                                                    semantic_error(program, item5, "non-class as heritage, for (%lld:%lld)",
                                                        heritage4->type->position.line, heritage4->type->position.column);
                                                    return -1;
                                                }

                                                break;
                                            }
                                        }

                                        list_destroy(response4);
                                    }
                                    else
                                    {
                                        node_t *item5 = heritage4->value_update;

                                        node_class_t *class5 = (node_class_t *)item5->value;
                                        node_t *node5 = class5->block;
                                        node_block_t *block5 = (node_block_t *)node5->value;

                                        ilist_t *a6;
                                        for (a6 = block5->list->begin;a6 != block5->list->end;a6 = a6->next)
                                        {
                                            node_t *item6 = (node_t *)a6->value;
                                            if (item6->kind == NODE_KIND_CLASS)
                                            {
                                                node_class_t *class6 = (node_class_t *)item6->value;
                                                if (semantic_idcmp(class6->key, basic->right) == 1)
                                                {
                                                    if ((item2->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                    {
                                                        if ((class6->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                        {
                                                            semantic_error(program, item6, "private access");
                                                            return -1;
                                                        }

                                                        if ((class6->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                        {
                                                            semantic_error(program, item6, "protect access");
                                                            return -1;
                                                        }

                                                        item6->flag |= NODE_FLAG_DERIVE;

                                                        ilist_t *r6 = list_rpush(response, item6);
                                                        if (r6 == NULL)
                                                        {
                                                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                        return 1;
                                                    }
                                                    else
                                                    {
                                                        item6->flag &= ~NODE_FLAG_DERIVE;

                                                        ilist_t *r6 = list_rpush(response, item6);
                                                        if (r6 == NULL)
                                                        {
                                                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                        return 1;
                                                    }
                                                }
                                            }
                                            else
                                            if (item6->kind == NODE_KIND_ENUM)
                                            {
                                                node_enum_t *enum1 = (node_enum_t *)item6->value;
                                                if (semantic_idcmp(enum1->key, basic->right) == 1)
                                                {
                                                    if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                    {
                                                        if ((enum1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                        {
                                                            semantic_error(program, item6, "private access");
                                                            return -1;
                                                        }

                                                        if ((enum1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                        {
                                                            semantic_error(program, item6, "protect access");
                                                            return -1;
                                                        }

                                                        item6->flag |= NODE_FLAG_DERIVE;

                                                        ilist_t *r6 = list_rpush(response, item6);
                                                        if (r6 == NULL)
                                                        {
                                                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                        return 1;
                                                    }
                                                    else
                                                    {
                                                        item6->flag &= ~NODE_FLAG_DERIVE;

                                                        ilist_t *r6 = list_rpush(response, item6);
                                                        if (r6 == NULL)
                                                        {
                                                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                        return 1;
                                                    }
                                                }
                                            }
                                        }

                                        ilist_t *r3 = list_rpush(repository1, class5->heritages);
                                        if (r3 == NULL)
                                        {
                                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }
                                }
                            }
                        
                            ilist_t *r3 = list_rpop(repository1);
                            if (r3 != NULL)
                            {
                                root = (node_t *)r3->value;
                                continue;
                            }
                            else
                            {
                                root = NULL;
                                break;
                            }
                        }

                        list_destroy(repository1);
                    }
                }
                else
                {
                    semantic_error(program, item2, "not a class, in (%lld:%lld)",
                        node->position.line, node->position.column);
                    return -1;
                }
            }
            else
            if (item1->kind == NODE_KIND_PROPERTY)
            {
                node_property_t *property1 = (node_property_t *)item1;
                if (property1->value_update == NULL)
                {
                    semantic_error(program, item1, "not initialized");
                    return -1;
                }

                node_t *item2 = property1->value_update;
                if (item2->kind == NODE_KIND_CLASS)
                {
                    node_class_t *class1 = (node_class_t *)item2->value;
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
                                        semantic_error(program, item2, "private access");
                                        return -1;
                                    }

                                    if ((class2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                    {
                                        semantic_error(program, item1, "protect access");
                                        return -1;
                                    }

                                    item2->flag |= NODE_FLAG_DERIVE;

                                    ilist_t *r2 = list_rpush(response, item2);
                                    if (r2 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    list_destroy(response1);
                                    return 1;
                                }
                                else
                                {
                                    item2->flag &= ~NODE_FLAG_DERIVE;

                                    ilist_t *r2 = list_rpush(response, item2);
                                    if (r2 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    list_destroy(response1);
                                    return 1;
                                }
                            }
                        }
                        else
                        if (item2->kind == NODE_KIND_ENUM)
                        {
                            node_enum_t *enum1 = (node_enum_t *)item2->value;
                            if (semantic_idcmp(basic->right, enum1->key) == 1)
                            {
                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                {
                                    if ((enum1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                    {
                                        semantic_error(program, item2, "private access");
                                        return -1;
                                    }

                                    if ((enum1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                    {
                                        semantic_error(program, item1, "protect access");
                                        return -1;
                                    }

                                    item2->flag |= NODE_FLAG_DERIVE;

                                    ilist_t *r2 = list_rpush(response, item2);
                                    if (r2 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    list_destroy(response1);
                                    return 1;
                                }
                                else
                                {
                                    item2->flag &= ~NODE_FLAG_DERIVE;
                                    
                                    ilist_t *r2 = list_rpush(response, item2);
                                    if (r2 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    list_destroy(response1);
                                    return 1;
                                }
                            }
                        }
                    }

                    if (class1->heritages != NULL)
                    {
                        node_t *root = class1->heritages;
                        list_t *repository1 = list_create();
                        if (repository1 == NULL)
                        {
                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }

                        while (root != NULL)
                        {
                            node_block_t *block3 = (node_block_t *)root->value;

                            ilist_t *a3;
                            for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                            {
                                node_t *item3 = (node_t *)a3->value;
                                
                                if (item3->kind == NODE_KIND_HERITAGE)
                                {
                                    node_heritage_t *heritage4 = (node_heritage_t *)item3->value;
                                    
                                    if (heritage4->value_update == NULL)
                                    {
                                        list_t *response4 = list_create();
                                        if (response4 == NULL)
                                        {
                                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }
                                        
                                        int32_t r4 = semantic_postfix(program, heritage4->type, response4);
                                        if (r4 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r4 == 0)
                                        {
                                            semantic_error(program, heritage4->type, "reference not found");
                                            return -1;
                                        }
                                        else
                                        if (r4 == 1)
                                        {
                                            ilist_t *a5;
                                            for (a5 = response4->begin;a5 != response4->end;a5 = a5->next)
                                            {
                                                node_t *item5 = (node_t *)a5->value;
                                                
                                                heritage4->value_update = item5;

                                                if (item5->kind == NODE_KIND_CLASS)
                                                {
                                                    node_class_t *class5 = (node_class_t *)item5->value;
                                                    node_t *node5 = class5->block;
                                                    node_block_t *block5 = (node_block_t *)node5->value;

                                                    ilist_t *a6;
                                                    for (a6 = block5->list->begin;a6 != block5->list->end;a6 = a6->next)
                                                    {
                                                        node_t *item6 = (node_t *)a6->value;
                                                        if (item6->kind == NODE_KIND_CLASS)
                                                        {
                                                            node_class_t *class6 = (node_class_t *)item6->value;
                                                            if (semantic_idcmp(class6->key, basic->right) == 1)
                                                            {
                                                                if ((item2->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                {
                                                                    if ((class6->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                    {
                                                                        semantic_error(program, item6, "private access");
                                                                        return -1;
                                                                    }

                                                                    if ((class6->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                    {
                                                                        semantic_error(program, item6, "protect access");
                                                                        return -1;
                                                                    }

                                                                    item6->flag |= NODE_FLAG_DERIVE;

                                                                    ilist_t *r6 = list_rpush(response, item6);
                                                                    if (r6 == NULL)
                                                                    {
                                                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                                        return -1;
                                                                    }
                                                                    list_destroy(response4);
                                                                    return 1;
                                                                }
                                                                else
                                                                {
                                                                    item6->flag &= ~NODE_FLAG_DERIVE;

                                                                    ilist_t *r6 = list_rpush(response, item6);
                                                                    if (r6 == NULL)
                                                                    {
                                                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                                        return -1;
                                                                    }
                                                                    list_destroy(response4);
                                                                    return 1;
                                                                }
                                                            }
                                                        }
                                                        else
                                                        if (item6->kind == NODE_KIND_ENUM)
                                                        {
                                                            node_enum_t *enum1 = (node_enum_t *)item6->value;
                                                            if (semantic_idcmp(enum1->key, basic->right) == 1)
                                                            {
                                                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                {
                                                                    if ((enum1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                    {
                                                                        semantic_error(program, item6, "private access");
                                                                        return -1;
                                                                    }

                                                                    if ((enum1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                    {
                                                                        semantic_error(program, item6, "protect access");
                                                                        return -1;
                                                                    }

                                                                    item6->flag |= NODE_FLAG_DERIVE;

                                                                    ilist_t *r6 = list_rpush(response, item6);
                                                                    if (r6 == NULL)
                                                                    {
                                                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                                        return -1;
                                                                    }
                                                                    list_destroy(response4);
                                                                    return 1;
                                                                }
                                                                else
                                                                {
                                                                    item6->flag &= ~NODE_FLAG_DERIVE;

                                                                    ilist_t *r6 = list_rpush(response, item6);
                                                                    if (r6 == NULL)
                                                                    {
                                                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                                        return -1;
                                                                    }
                                                                    list_destroy(response4);
                                                                    return 1;
                                                                }
                                                            }
                                                        }
                                                    }

                                                    ilist_t *r3 = list_rpush(repository1, class5->heritages);
                                                    if (r3 == NULL)
                                                    {
                                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                }
                                                else
                                                {
                                                    semantic_error(program, item5, "non-class as heritage, for (%lld:%lld)",
                                                        heritage4->type->position.line, heritage4->type->position.column);
                                                    return -1;
                                                }

                                                break;
                                            }
                                        }

                                        list_destroy(response4);
                                    }
                                    else
                                    {
                                        node_t *item5 = heritage4->value_update;

                                        node_class_t *class5 = (node_class_t *)item5->value;
                                        node_t *node5 = class5->block;
                                        node_block_t *block5 = (node_block_t *)node5->value;

                                        ilist_t *a6;
                                        for (a6 = block5->list->begin;a6 != block5->list->end;a6 = a6->next)
                                        {
                                            node_t *item6 = (node_t *)a6->value;
                                            if (item6->kind == NODE_KIND_CLASS)
                                            {
                                                node_class_t *class6 = (node_class_t *)item6->value;
                                                if (semantic_idcmp(class6->key, basic->right) == 1)
                                                {
                                                    if ((item2->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                    {
                                                        if ((class6->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                        {
                                                            semantic_error(program, item6, "private access");
                                                            return -1;
                                                        }

                                                        if ((class6->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                        {
                                                            semantic_error(program, item6, "protect access");
                                                            return -1;
                                                        }

                                                        item6->flag |= NODE_FLAG_DERIVE;

                                                        ilist_t *r6 = list_rpush(response, item6);
                                                        if (r6 == NULL)
                                                        {
                                                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                        return 1;
                                                    }
                                                    else
                                                    {
                                                        item6->flag &= ~NODE_FLAG_DERIVE;

                                                        ilist_t *r6 = list_rpush(response, item6);
                                                        if (r6 == NULL)
                                                        {
                                                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                        return 1;
                                                    }
                                                }
                                            }
                                            else
                                            if (item6->kind == NODE_KIND_ENUM)
                                            {
                                                node_enum_t *enum1 = (node_enum_t *)item6->value;
                                                if (semantic_idcmp(enum1->key, basic->right) == 1)
                                                {
                                                    if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                    {
                                                        if ((enum1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                        {
                                                            semantic_error(program, item6, "private access");
                                                            return -1;
                                                        }

                                                        if ((enum1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                        {
                                                            semantic_error(program, item6, "protect access");
                                                            return -1;
                                                        }

                                                        item6->flag |= NODE_FLAG_DERIVE;

                                                        ilist_t *r6 = list_rpush(response, item6);
                                                        if (r6 == NULL)
                                                        {
                                                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                        return 1;
                                                    }
                                                    else
                                                    {
                                                        item6->flag &= ~NODE_FLAG_DERIVE;

                                                        ilist_t *r6 = list_rpush(response, item6);
                                                        if (r6 == NULL)
                                                        {
                                                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                        return 1;
                                                    }
                                                }
                                            }
                                        }

                                        ilist_t *r3 = list_rpush(repository1, class5->heritages);
                                        if (r3 == NULL)
                                        {
                                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }
                                }
                            }
                        
                            ilist_t *r3 = list_rpop(repository1);
                            if (r3 != NULL)
                            {
                                root = (node_t *)r3->value;
                                continue;
                            }
                            else
                            {
                                root = NULL;
                                break;
                            }
                        }

                        list_destroy(repository1);
                    }
                }
                else
                {
                    semantic_error(program, item2, "not a class, in (%lld:%lld)",
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
                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                
                    int32_t r2 = semantic_postfix(program, heritage1->type, response2);
                    if (r2 == -1)
                    {
                        return -1;
                    }
                    else
                    if (r2 == 0)
                    {
                        semantic_error(program, heritage1->type, "reference not found");
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
                                heritage1->value_update = item2;

                                node_class_t *class2 = (node_class_t *)item2->value;
                                node_t *node2 = class2->block;
                                node_block_t *block2 = (node_block_t *)node2->value;

                                ilist_t *a3;
                                for (a3 = block2->list->begin;a3 != block2->list->end;a3 = a3->next)
                                {
                                    node_t *item3 = (node_t *)a3->value;
                                    if (item3->kind == NODE_KIND_CLASS)
                                    {
                                        node_class_t *class3 = (node_class_t *)item3->value;
                                        if (semantic_idcmp(class3->key, basic->right) == 1)
                                        {
                                            if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                            {
                                                if ((class3->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                {
                                                    semantic_error(program, item3, "private access");
                                                    return -1;
                                                }

                                                if ((class3->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                {
                                                    semantic_error(program, item3, "protect access");
                                                    return -1;
                                                }

                                                item3->flag |= NODE_FLAG_DERIVE;

                                                ilist_t *r3 = list_rpush(response, item3);
                                                if (r3 == NULL)
                                                {
                                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                list_destroy(response2);
                                                return 1;
                                            }
                                            else
                                            {
                                                item3->flag &= ~NODE_FLAG_DERIVE;

                                                ilist_t *r3 = list_rpush(response, item3);
                                                if (r3 == NULL)
                                                {
                                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                list_destroy(response2);
                                                return 1;
                                            }
                                        }
                                    }
                                    else
                                    if (item3->kind == NODE_KIND_ENUM)
                                    {
                                        node_enum_t *enum3 = (node_enum_t *)item3->value;
                                        if (semantic_idcmp(enum3->key, basic->right) == 1)
                                        {
                                            if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                            {
                                                if ((enum3->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                {
                                                    semantic_error(program, item3, "private access");
                                                    return -1;
                                                }

                                                if ((enum3->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                {
                                                    semantic_error(program, item3, "protect access");
                                                    return -1;
                                                }

                                                item3->flag |= NODE_FLAG_DERIVE;

                                                ilist_t *r3 = list_rpush(response, item3);
                                                if (r3 == NULL)
                                                {
                                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                list_destroy(response2);
                                                return 1;
                                            }
                                            else
                                            {
                                                item3->flag &= ~NODE_FLAG_DERIVE;

                                                ilist_t *r3 = list_rpush(response, item3);
                                                if (r3 == NULL)
                                                {
                                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                list_destroy(response2);
                                                return 1;
                                            }
                                        }
                                    }
                                }

                                if (class2->heritages != NULL)
                                {
                                    node_t *root = class2->heritages;
                                    list_t *repository1 = list_create();
                                    if (repository1 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    while (root != NULL)
                                    {
                                        node_block_t *block4 = (node_block_t *)root->value;

                                        ilist_t *a4;
                                        for (a4 = block4->list->begin;a4 != block4->list->end;a4 = a4->next)
                                        {
                                            node_t *item4 = (node_t *)a4->value;
                                            
                                            if (item4->kind == NODE_KIND_HERITAGE)
                                            {
                                                node_heritage_t *heritage4 = (node_heritage_t *)item4->value;

                                                list_t *response4 = list_create();
                                                if (response4 == NULL)
                                                {
                                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                            
                                                int32_t r4 = semantic_postfix(program, heritage4->type, response4);
                                                if (r4 == -1)
                                                {
                                                    return -1;
                                                }
                                                else
                                                if (r4 == 0)
                                                {
                                                    semantic_error(program, heritage4->type, "reference not found");
                                                    return -1;
                                                }
                                                else
                                                if (r4 == 1)
                                                {
                                                    ilist_t *a5;
                                                    for (a5 = response4->begin;a5 != response4->end;a5 = a5->next)
                                                    {
                                                        node_t *item5 = (node_t *)a5->value;

                                                        if (item5->kind == NODE_KIND_CLASS)
                                                        {
                                                            heritage4->value_update = item5;

                                                            node_class_t *class5 = (node_class_t *)item5->value;
                                                            node_t *node5 = class5->block;
                                                            node_block_t *block5 = (node_block_t *)node5->value;

                                                            ilist_t *a6;
                                                            for (a6 = block5->list->begin;a6 != block5->list->end;a6 = a6->next)
                                                            {
                                                                node_t *item6 = (node_t *)a6->value;
                                                                if (item6->kind == NODE_KIND_CLASS)
                                                                {
                                                                    node_class_t *class6 = (node_class_t *)item6->value;
                                                                    if (semantic_idcmp(class6->key, basic->right) == 1)
                                                                    {
                                                                        if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                        {
                                                                            if ((class6->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                            {
                                                                                semantic_error(program, item6, "private access");
                                                                                return -1;
                                                                            }

                                                                            if ((class6->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                            {
                                                                                semantic_error(program, item6, "protect access");
                                                                                return -1;
                                                                            }

                                                                            item6->flag |= NODE_FLAG_DERIVE;

                                                                            ilist_t *r6 = list_rpush(response, item6);
                                                                            if (r6 == NULL)
                                                                            {
                                                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                                                return -1;
                                                                            }
                                                                            list_destroy(response4);
                                                                            return 1;
                                                                        }
                                                                        else
                                                                        {
                                                                            item6->flag &= ~NODE_FLAG_DERIVE;

                                                                            ilist_t *r6 = list_rpush(response, item6);
                                                                            if (r6 == NULL)
                                                                            {
                                                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                                                return -1;
                                                                            }
                                                                            list_destroy(response4);
                                                                            return 1;
                                                                        }
                                                                    }
                                                                }
                                                                else
                                                                if (item6->kind == NODE_KIND_ENUM)
                                                                {
                                                                    node_enum_t *enum6 = (node_enum_t *)item6->value;
                                                                    if (semantic_idcmp(enum6->key, basic->right) == 1)
                                                                    {
                                                                        if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                        {
                                                                            if ((enum6->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                            {
                                                                                semantic_error(program, item6, "private access");
                                                                                return -1;
                                                                            }

                                                                            if ((enum6->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                            {
                                                                                semantic_error(program, item6, "protect access");
                                                                                return -1;
                                                                            }

                                                                            item6->flag |= NODE_FLAG_DERIVE;

                                                                            ilist_t *r6 = list_rpush(response, item6);
                                                                            if (r6 == NULL)
                                                                            {
                                                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                                                return -1;
                                                                            }
                                                                            list_destroy(response4);
                                                                            return 1;
                                                                        }
                                                                        else
                                                                        {
                                                                            item6->flag &= ~NODE_FLAG_DERIVE;

                                                                            ilist_t *r6 = list_rpush(response, item6);
                                                                            if (r6 == NULL)
                                                                            {
                                                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                                                return -1;
                                                                            }
                                                                            list_destroy(response4);
                                                                            return 1;
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        
                                                            ilist_t *r6 = list_rpush(repository1, class5->heritages);
                                                            if (r6 == NULL)
                                                            {
                                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                                return -1;
                                                            }
                                                        }
                                                        else
                                                        {
                                                            semantic_error(program, item5, "non-class as heritage, for (%lld:%lld)",
                                                                heritage4->type->position.line, heritage4->type->position.column);
                                                            return -1;
                                                        }
                                                    }
                                                }
                                            
                                            }
                                        }
                                    
                                        ilist_t *r4 = list_rpop(repository1);
                                        if (r4 != NULL)
                                        {
                                            root = (node_t *)r4->value;
                                            continue;
                                        }
                                        else
                                        {
                                            root = NULL;
                                            break;
                                        }
                                    }

                                    list_destroy(repository1);
                                }
                            }
                            else
                            {
                                semantic_error(program, item2, "non-class as heritage, for (%lld:%lld)",
                                    heritage1->type->position.line, heritage1->type->position.column);
                                return -1;
                            }
                            break;
                        }
                    }

                    list_destroy(response2);
                }
                else
                {
                    node_t *item2 = heritage1->value_update;

                    node_class_t *class2 = (node_class_t *)item2->value;
                    node_t *node2 = class2->block;
                    node_block_t *block2 = (node_block_t *)node2->value;

                    ilist_t *a3;
                    for (a3 = block2->list->begin;a3 != block2->list->end;a3 = a3->next)
                    {
                        node_t *item3 = (node_t *)a3->value;
                        if (item3->kind == NODE_KIND_CLASS)
                        {
                            node_class_t *class3 = (node_class_t *)item3->value;
                            if (semantic_idcmp(class3->key, basic->right) == 1)
                            {
                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                {
                                    if ((class3->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                    {
                                        semantic_error(program, item3, "private access");
                                        return -1;
                                    }

                                    if ((class3->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                    {
                                        semantic_error(program, item3, "protect access");
                                        return -1;
                                    }

                                    item3->flag |= NODE_FLAG_DERIVE;

                                    ilist_t *r3 = list_rpush(response, item3);
                                    if (r3 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    return 1;
                                }
                                else
                                {
                                    item3->flag &= ~NODE_FLAG_DERIVE;

                                    ilist_t *r3 = list_rpush(response, item3);
                                    if (r3 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    return 1;
                                }
                            }
                        }
                        else
                        if (item3->kind == NODE_KIND_ENUM)
                        {
                            node_enum_t *enum3 = (node_enum_t *)item3->value;
                            if (semantic_idcmp(enum3->key, basic->right) == 1)
                            {
                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                {
                                    if ((enum3->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                    {
                                        semantic_error(program, item3, "private access");
                                        return -1;
                                    }

                                    if ((enum3->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                    {
                                        semantic_error(program, item3, "protect access");
                                        return -1;
                                    }

                                    item3->flag |= NODE_FLAG_DERIVE;

                                    ilist_t *r3 = list_rpush(response, item3);
                                    if (r3 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    return 1;
                                }
                                else
                                {
                                    item3->flag &= ~NODE_FLAG_DERIVE;

                                    ilist_t *r3 = list_rpush(response, item3);
                                    if (r3 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    return 1;
                                }
                            }
                        }
                    }

                    if (class2->heritages != NULL)
                    {
                        node_t *root = class2->heritages;
                        list_t *repository1 = list_create();
                        if (repository1 == NULL)
                        {
                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }

                        while (root != NULL)
                        {
                            node_block_t *block4 = (node_block_t *)root->value;

                            ilist_t *a4;
                            for (a4 = block4->list->begin;a4 != block4->list->end;a4 = a4->next)
                            {
                                node_t *item4 = (node_t *)a4->value;
                                
                                if (item4->kind == NODE_KIND_HERITAGE)
                                {
                                    node_heritage_t *heritage4 = (node_heritage_t *)item4->value;

                                    list_t *response4 = list_create();
                                    if (response4 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                
                                    int32_t r4 = semantic_postfix(program, heritage4->type, response4);
                                    if (r4 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r4 == 0)
                                    {
                                        semantic_error(program, heritage4->type, "reference not found");
                                        return -1;
                                    }
                                    else
                                    if (r4 == 1)
                                    {
                                        ilist_t *a5;
                                        for (a5 = response4->begin;a5 != response4->end;a5 = a5->next)
                                        {
                                            node_t *item5 = (node_t *)a5->value;

                                            if (item5->kind == NODE_KIND_CLASS)
                                            {
                                                heritage4->value_update = item5;

                                                node_class_t *class5 = (node_class_t *)item5->value;
                                                node_t *node5 = class5->block;
                                                node_block_t *block5 = (node_block_t *)node5->value;

                                                ilist_t *a6;
                                                for (a6 = block5->list->begin;a6 != block5->list->end;a6 = a6->next)
                                                {
                                                    node_t *item6 = (node_t *)a6->value;
                                                    if (item6->kind == NODE_KIND_CLASS)
                                                    {
                                                        node_class_t *class6 = (node_class_t *)item6->value;
                                                        if (semantic_idcmp(class6->key, basic->right) == 1)
                                                        {
                                                            if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                            {
                                                                if ((class6->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                {
                                                                    semantic_error(program, item6, "private access");
                                                                    return -1;
                                                                }

                                                                if ((class6->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                {
                                                                    semantic_error(program, item6, "protect access");
                                                                    return -1;
                                                                }

                                                                item6->flag |= NODE_FLAG_DERIVE;

                                                                ilist_t *r6 = list_rpush(response, item6);
                                                                if (r6 == NULL)
                                                                {
                                                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                                    return -1;
                                                                }
                                                                list_destroy(response4);
                                                                return 1;
                                                            }
                                                            else
                                                            {
                                                                item6->flag &= ~NODE_FLAG_DERIVE;

                                                                ilist_t *r6 = list_rpush(response, item6);
                                                                if (r6 == NULL)
                                                                {
                                                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                                    return -1;
                                                                }
                                                                list_destroy(response4);
                                                                return 1;
                                                            }
                                                        }
                                                    }
                                                    else
                                                    if (item6->kind == NODE_KIND_ENUM)
                                                    {
                                                        node_enum_t *enum6 = (node_enum_t *)item6->value;
                                                        if (semantic_idcmp(enum6->key, basic->right) == 1)
                                                        {
                                                            if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                            {
                                                                if ((enum6->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                {
                                                                    semantic_error(program, item6, "private access");
                                                                    return -1;
                                                                }

                                                                if ((enum6->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                {
                                                                    semantic_error(program, item6, "protect access");
                                                                    return -1;
                                                                }

                                                                item6->flag |= NODE_FLAG_DERIVE;

                                                                ilist_t *r6 = list_rpush(response, item6);
                                                                if (r6 == NULL)
                                                                {
                                                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                                    return -1;
                                                                }
                                                                list_destroy(response4);
                                                                return 1;
                                                            }
                                                            else
                                                            {
                                                                item6->flag &= ~NODE_FLAG_DERIVE;

                                                                ilist_t *r6 = list_rpush(response, item6);
                                                                if (r6 == NULL)
                                                                {
                                                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                                    return -1;
                                                                }
                                                                list_destroy(response4);
                                                                return 1;
                                                            }
                                                        }
                                                    }
                                                }
                                            
                                                ilist_t *r6 = list_rpush(repository1, class5->heritages);
                                                if (r6 == NULL)
                                                {
                                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                            }
                                            else
                                            {
                                                semantic_error(program, item5, "non-class as heritage, for (%lld:%lld)",
                                                    heritage4->type->position.line, heritage4->type->position.column);
                                                return -1;
                                            }
                                        }
                                    }
                                
                                }
                            }
                        
                            ilist_t *r4 = list_rpop(repository1);
                            if (r4 != NULL)
                            {
                                root = (node_t *)r4->value;
                                continue;
                            }
                            else
                            {
                                root = NULL;
                                break;
                            }
                        }

                        list_destroy(repository1);
                    }
                }
            }
        }
    }

    list_destroy(response1);

    return 0;
}

static int32_t
semantic_postfix(program_t *program, node_t *node, list_t *response)
{
    if (node->kind == NODE_KIND_ATTRIBUTE)
    {
        return semantic_attribute(program, node, response);
    }
    else
    if (node->kind == NODE_KIND_PSEUDONYM)
    {
        return semantic_pseudonym(program, node, response);
    }
    else
    {
        return semantic_primary(program, node, response);
    }
}



static int32_t
semantic_if(program_t *program, node_t *node)
{
	return 1;
}

static int32_t
semantic_for(program_t *program, node_t *node)
{
    node_for_t *for1 = (node_for_t *)node->value;

    if (for1->key != NULL)
    {
        node_t *sub = node;
        node_t *current = node->parent;
        while (current != NULL)
        {
            if (current->kind == NODE_KIND_TRY)
            {
                node_try_t *try2 = (node_try_t *)current->value;
                node_t *node2 = try2->generics;
                node_block_t *block2 = node2->value;
                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    if (item2->kind == NODE_KIND_GENERIC)
                    {
                        node_generic_t *generic2 = (node_generic_t *)item2->value;
                        if (semantic_idcmp(for1->key, generic2->key) == 1)
                        {
                            semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                generic2->key->position.line, generic2->key->position.column);
                            return -1;
                        }
                    }
                }
                break;
            }
            else
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
                                if (item3->kind == NODE_KIND_PROPERTY)
                                {
                                    node_property_t *property1 = (node_property_t *)item3->value;
                                    if (semantic_idcmp(for1->key, property1->key) == 1)
                                    {
                                        semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                            property1->key->position.line, property1->key->position.column);
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
                                if (item3->kind == NODE_KIND_PROPERTY)
                                {
                                    node_property_t *property1 = (node_property_t *)item3->value;
                                    if (semantic_idcmp(for1->key, property1->key) == 1)
                                    {
                                        semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                            property1->key->position.line, property1->key->position.column);
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
                    if (item2->kind == NODE_KIND_TRY)
                    {
                        node_try_t *try2 = (node_try_t *)item2->value;
                        if (semantic_idcmp(for1->key, try2->key) == 1)
                        {
                            node_t *ngs1 = try2->generics;
                            node_t *ngs2 = NULL;
                            int32_t r1 = semantic_eqaul_gsgs(program, ngs1, ngs2);
                            if (r1 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r1 == 1)
                            {
                                semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                    try2->key->position.line, try2->key->position.column);
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
                                if (item3->kind == NODE_KIND_PROPERTY)
                                {
                                    node_property_t *property1 = (node_property_t *)item3->value;
                                    if (semantic_idcmp(for1->key, property1->key) == 1)
                                    {
                                        semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                            property1->key->position.line, property1->key->position.column);
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
                node_fun_t *func2 = (node_fun_t *)current->value;

                if (func2->generics != NULL)
                {
                    node_t *node2 = func2->generics;
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

                if (func2->parameters != NULL)
                {
                    node_t *node3 = func2->parameters;
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
semantic_forin(program_t *program, node_t *node)
{
    node_forin_t *for1 = (node_forin_t *)node->value;

    if (for1->key != NULL)
    {
        node_t *sub = node;
        node_t *current = node->parent;
        while (current != NULL)
        {
            if (current->kind == NODE_KIND_TRY)
            {
                node_try_t *try2 = (node_try_t *)current->value;
                node_t *node2 = try2->generics;
                node_block_t *block2 = node2->value;
                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    if (item2->kind == NODE_KIND_GENERIC)
                    {
                        node_generic_t *generic2 = (node_generic_t *)item2->value;
                        if (semantic_idcmp(for1->key, generic2->key) == 1)
                        {
                            semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                generic2->key->position.line, generic2->key->position.column);
                            return -1;
                        }
                    }
                }
                break;
            }
            else
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
                                if (item3->kind == NODE_KIND_PROPERTY)
                                {
                                    node_property_t *property1 = (node_property_t *)item3->value;
                                    if (semantic_idcmp(for1->key, property1->key) == 1)
                                    {
                                        semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                            property1->key->position.line, property1->key->position.column);
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
                                if (item3->kind == NODE_KIND_PROPERTY)
                                {
                                    node_property_t *property1 = (node_property_t *)item3->value;
                                    if (semantic_idcmp(for1->key, property1->key) == 1)
                                    {
                                        semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                            property1->key->position.line, property1->key->position.column);
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
                    if (item2->kind == NODE_KIND_TRY)
                    {
                        node_try_t *try2 = (node_try_t *)item2->value;
                        if (semantic_idcmp(for1->key, try2->key) == 1)
                        {
                            node_t *ngs1 = try2->generics;
                            node_t *ngs2 = NULL;
                            int32_t r1 = semantic_eqaul_gsgs(program, ngs1, ngs2);
                            if (r1 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r1 == 1)
                            {
                                semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                    try2->key->position.line, try2->key->position.column);
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
                                if (item3->kind == NODE_KIND_PROPERTY)
                                {
                                    node_property_t *property1 = (node_property_t *)item3->value;
                                    if (semantic_idcmp(for1->key, property1->key) == 1)
                                    {
                                        semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                            property1->key->position.line, property1->key->position.column);
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
                node_fun_t *func2 = (node_fun_t *)current->value;

                if (func2->generics != NULL)
                {
                    node_t *node2 = func2->generics;
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

                if (func2->parameters != NULL)
                {
                    node_t *node3 = func2->parameters;
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
semantic_parameter(program_t *program, node_t *node)
{
    node_parameter_t *parameter1 = (node_parameter_t *)node->value;
    
    node_t *sub = node;
    node_t *current = node->parent;
    while (current != NULL)
    {
        if (current->kind == NODE_KIND_TRY)
        {
            node_try_t *try2 = (node_try_t *)current->value;
            node_t *node2 = try2->generics;
            node_block_t *block2 = node2->value;
            ilist_t *a2;
            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->kind == NODE_KIND_GENERIC)
                {
                    node_generic_t *generic2 = (node_generic_t *)item2->value;
                    if (semantic_idcmp(parameter1->key, generic2->key) == 1)
                    {
                        semantic_error(program, parameter1->key, "already defined, previous in (%lld:%lld)",
                            generic2->key->position.line, generic2->key->position.column);
                        return -1;
                    }
                }
            }
            break;
        }
        else
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
            node_fun_t *func2 = (node_fun_t *)current->value;

            if (func2->generics != NULL)
            {
                node_t *node2 = func2->generics;
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

    list_t *response1 = list_create();
    if (response1 == NULL)
    {
        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    int32_t r1 = semantic_postfix(program, parameter1->type, response1);
    if (r1 == -1)
    {
        return -1;
    }
    else
    if (r1 == 0)
    {
        semantic_error(program, parameter1->type, "reference not found");
        return -1;
    }

    list_destroy(response1);

    return 1;
}

static int32_t
semantic_parameters(program_t *program, node_t *node)
{
	node_block_t *parameters = (node_block_t *)node->value;
    ilist_t *a1;
    for (a1 = parameters->list->begin;a1 != parameters->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        int32_t result;
        result = semantic_parameter(program, item1);
        if (result == -1)
        {
            return -1;
        }
    }
	return 1;
}

static int32_t
semantic_generic(program_t *program, node_t *node)
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

    if (generic1->type != NULL)
    {
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_postfix(program, generic1->type, response1);
        if (r1 == -1)
        {
            return -1;
        }
        else
        if (r1 == 0)
        {
            semantic_error(program, generic1->type, "reference not found");
            return -1;
        }

        list_destroy(response1);
    }

    return 1;
}

static int32_t
semantic_generics(program_t *program, node_t *node)
{
	node_block_t *generics = (node_block_t *)node->value;
    ilist_t *a1;
    for (a1 = generics->list->begin;a1 != generics->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        int32_t result;
        result = semantic_generic(program, item1);
        if (result == -1)
        {
            return -1;
        }
    }
	return 1;
}

static int32_t
semantic_catch(program_t *program, node_t *node)
{
    node_catch_t *catch1 = (node_catch_t *)node->value;

    return 1;
}

static int32_t
semantic_try(program_t *program, node_t *node)
{
	node_try_t *try1 = (node_try_t *)node->value;

    node_t *sub = node;
    node_t *current = node->parent;
    while (current != NULL)
    {
        if (current->kind == NODE_KIND_TRY)
        {
            node_try_t *try2 = (node_try_t *)current->value;
            node_t *node2 = try2->generics;
            node_block_t *block2 = node2->value;
            ilist_t *a2;
            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->kind == NODE_KIND_GENERIC)
                {
                    node_generic_t *generic2 = (node_generic_t *)item2->value;
                    if (semantic_idcmp(try1->key, generic2->key) == 1)
                    {
                        semantic_error(program, try1->key, "already defined, previous in (%lld:%lld)",
                            generic2->key->position.line, generic2->key->position.column);
                        return -1;
                    }
                }
            }
            break;
        }
        else
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
                    if (semantic_idcmp(try1->key, parameter2->key) == 1)
                    {
                        semantic_error(program, try1->key, "already defined, previous in (%lld:%lld)",
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
                        if (semantic_idcmp(try1->key, var2->key) == 1)
                        {
                            semantic_error(program, try1->key, "already defined, previous in (%lld:%lld)",
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
                            if (item3->kind == NODE_KIND_PROPERTY)
                            {
                                node_property_t *property1 = (node_property_t *)item3->value;
                                if (semantic_idcmp(try1->key, property1->key) == 1)
                                {
                                    semantic_error(program, try1->key, "already defined, previous in (%lld:%lld)",
                                        property1->key->position.line, property1->key->position.column);
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
                        if (semantic_idcmp(try1->key, var2->key) == 1)
                        {
                            semantic_error(program, node, "already defined, previous in (%lld:%lld)",
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
                            if (item3->kind == NODE_KIND_PROPERTY)
                            {
                                node_property_t *property1 = (node_property_t *)item3->value;
                                if (semantic_idcmp(try1->key, property1->key) == 1)
                                {
                                    semantic_error(program, try1->key, "already defined, previous in (%lld:%lld)",
                                        property1->key->position.line, property1->key->position.column);
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
                        if (semantic_idcmp(try1->key, for2->key) == 1)
                        {
                            semantic_error(program, try1->key, "already defined, previous in (%lld:%lld)",
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
                        if (semantic_idcmp(try1->key, for2->key) == 1)
                        {
                            semantic_error(program, try1->key, "already defined, previous in (%lld:%lld)",
                                for2->key->position.line, for2->key->position.column);
                            return -1;
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_TRY)
                {
                    node_try_t *try2 = (node_try_t *)item2->value;
                    if (semantic_idcmp(try1->key, try2->key) == 1)
                    {
                        node_t *ngs1 = try1->generics;
                        node_t *ngs2 = try2->generics;
                        int32_t r1 = semantic_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            semantic_error(program, try1->key, "already defined, previous in (%lld:%lld)",
                                try2->key->position.line, try2->key->position.column);
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
                        if (semantic_idcmp(try1->key, var2->key) == 1)
                        {
                            semantic_error(program, try1->key, "already defined, previous in (%lld:%lld)",
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
                            if (item3->kind == NODE_KIND_PROPERTY)
                            {
                                node_property_t *property1 = (node_property_t *)item3->value;
                                if (semantic_idcmp(try1->key, property1->key) == 1)
                                {
                                    semantic_error(program, try1->key, "already defined, previous in (%lld:%lld)",
                                        property1->key->position.line, property1->key->position.column);
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
        if (current->kind == NODE_KIND_FUN)
        {
            node_fun_t *func2 = (node_fun_t *)current->value;

            if (func2->generics != NULL)
            {
                node_t *node2 = func2->generics;
                node_block_t *block2 = (node_block_t *)node2->value;

                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;

                    if (item2->kind == NODE_KIND_GENERIC)
                    {
                        node_generic_t *generic3 = (node_generic_t *)item2->value;
                        if (semantic_idcmp(try1->key, generic3->key) == 1)
                        {
                            semantic_error(program, try1->key, "already defined, previous in (%lld:%lld)",
                                generic3->key->position.line, generic3->key->position.column);
                            return -1;
                        }
                    }
                }
            }

            if (func2->parameters != NULL)
            {
                node_t *node3 = func2->parameters;
                node_block_t *block3 = (node_block_t *)node3->value;

                ilist_t *a3;
                for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                {
                    node_t *item3 = (node_t *)a3->value;

                    if (item3->kind == NODE_KIND_PARAMETER)
                    {
                        node_parameter_t *parameter3 = (node_parameter_t *)item3->value;
                        if (semantic_idcmp(try1->key, parameter3->key) == 1)
                        {
                            semantic_error(program, try1->key, "already defined, previous in (%lld:%lld)",
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

    if (try1->generics != NULL)
    {
        int32_t r1 = semantic_generics(program, try1->generics);
        if (r1 == -1)
        {
            return -1;
        }
    }

    node_t *node2 = try1->catchs;
    node_block_t *block2 = (node_block_t *)node2->value;

    ilist_t *a1;
    for (a1 = block2->list->begin;a1 != block2->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        int32_t result;
        result = semantic_catch(program, item1);
        if (result == -1)
        {
            return -1;
        }
    }

	return 1;
}

static int32_t
semantic_var(program_t *program, node_t *node)
{
    node_var_t *var1 = (node_var_t *)node->value;

    node_t *sub = node;
    node_t *current = node->parent;
    while (current != NULL)
    {
        if (current->kind == NODE_KIND_TRY)
        {
            node_try_t *try2 = (node_try_t *)current->value;
            node_t *node2 = try2->generics;
            node_block_t *block2 = node2->value;
            ilist_t *a2;
            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->kind == NODE_KIND_GENERIC)
                {
                    node_generic_t *generic2 = (node_generic_t *)item2->value;
                    if (var1->key->kind == NODE_KIND_ID)
                    {
                        if (semantic_idcmp(var1->key, generic2->key) == 1)
                        {
                            semantic_error(program, var1->key, "already defined, previous in (%lld:%lld)",
                                generic2->key->position.line, generic2->key->position.column);
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
                            if (item4->kind == NODE_KIND_PROPERTY)
                            {
                                node_property_t *property4 = (node_property_t *)item4->value;
                                if (semantic_idcmp(property4->key, generic2->key) == 1)
                                {
                                    semantic_error(program, property4->key, "already defined, previous in (%lld:%lld)",
                                        generic2->key->position.line, generic2->key->position.column);
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
                            if (item4->kind == NODE_KIND_PROPERTY)
                            {
                                node_property_t *property4 = (node_property_t *)item4->value;
                                if (semantic_idcmp(property4->key, parameter2->key) == 1)
                                {
                                    semantic_error(program, property4->key, "already defined, previous in (%lld:%lld)",
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
                                if (item4->kind == NODE_KIND_PROPERTY)
                                {
                                    node_property_t *property4 = (node_property_t *)item4->value;
                                    if (semantic_idcmp(property4->key, var2->key) == 1)
                                    {
                                        semantic_error(program, property4->key, "already defined, previous in (%lld:%lld)",
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
                            if (item3->kind == NODE_KIND_PROPERTY)
                            {
                                node_property_t *property2 = (node_property_t *)item3->value;
                                if (var1->key->kind == NODE_KIND_ID)
                                {
                                    if (semantic_idcmp(var1->key, property2->key) == 1)
                                    {
                                        semantic_error(program, var1->key, "already defined, previous in (%lld:%lld)",
                                            property2->key->position.line, property2->key->position.column);
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
                                        if (item4->kind == NODE_KIND_PROPERTY)
                                        {
                                            node_property_t *property4 = (node_property_t *)item4->value;
                                            if (semantic_idcmp(property4->key, property2->key) == 1)
                                            {
                                                semantic_error(program, property4->key, "already defined, previous in (%lld:%lld)",
                                                    property2->key->position.line, property2->key->position.column);
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
                                if (item4->kind == NODE_KIND_PROPERTY)
                                {
                                    node_property_t *property4 = (node_property_t *)item4->value;
                                    if (semantic_idcmp(property4->key, var2->key) == 1)
                                    {
                                        semantic_error(program, property4->key, "already defined, previous in (%lld:%lld)",
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
                            if (item3->kind == NODE_KIND_PROPERTY)
                            {
                                node_property_t *property2 = (node_property_t *)item3->value;
                                if (var1->key->kind == NODE_KIND_ID)
                                {
                                    if (semantic_idcmp(var1->key, property2->key) == 1)
                                    {
                                        semantic_error(program, var1->key, "already defined, previous in (%lld:%lld)",
                                            property2->key->position.line, property2->key->position.column);
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
                                        if (item4->kind == NODE_KIND_PROPERTY)
                                        {
                                            node_property_t *property4 = (node_property_t *)item4->value;
                                            if (semantic_idcmp(property4->key, property2->key) == 1)
                                            {
                                                semantic_error(program, property4->key, "already defined, previous in (%lld:%lld)",
                                                    property2->key->position.line, property2->key->position.column);
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
                                if (item3->kind == NODE_KIND_PROPERTY)
                                {
                                    node_property_t *property1 = (node_property_t *)item3->value;
                                    if (semantic_idcmp(property1->key, for2->key) == 1)
                                    {
                                        semantic_error(program, property1->key, "already defined, previous in (%lld:%lld)",
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
                                if (item3->kind == NODE_KIND_PROPERTY)
                                {
                                    node_property_t *property1 = (node_property_t *)item3->value;
                                    if (semantic_idcmp(property1->key, for2->key) == 1)
                                    {
                                        semantic_error(program, property1->key, "already defined, previous in (%lld:%lld)",
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
                                if (item3->kind == NODE_KIND_PROPERTY)
                                {
                                    node_property_t *property1 = (node_property_t *)item3->value;
                                    if (semantic_idcmp(property1->key, var2->key) == 1)
                                    {
                                        semantic_error(program, property1->key, "already defined, previous in (%lld:%lld)",
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
                                if (item3->kind == NODE_KIND_PROPERTY)
                                {
                                    node_property_t *property1 = (node_property_t *)item3->value;
                                    if (semantic_idcmp(var1->key, property1->key) == 1)
                                    {
                                        semantic_error(program, var1->key, "already defined, previous in (%lld:%lld)",
                                            property1->key->position.line, property1->key->position.column);
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
                                if (item3->kind == NODE_KIND_PROPERTY)
                                {
                                    node_property_t *property1 = (node_property_t *)item3->value;
                                    node_t *node4 = (node_t *)var2->key;
                                    node_block_t *block4 = (node_block_t *)node4->value;
                                    ilist_t *a4;
                                    for (a4 = block4->list->begin;a4 != block4->list->end;a4 = a4->next)
                                    {
                                        node_t *item4 = (node_t *)a4->value;
                                        if (item4->kind == NODE_KIND_PROPERTY)
                                        {
                                            node_property_t *property2 = (node_property_t *)item4->value;
                                            if (semantic_idcmp(property1->key, property2->key) == 1)
                                            {
                                                semantic_error(program, property1->key, "already defined, previous in (%lld:%lld)",
                                                    property2->key->position.line, property2->key->position.column);
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
            node_fun_t *func2 = (node_fun_t *)current->value;

            if (func2->generics != NULL)
            {
                node_t *node2 = func2->generics;
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
                                
                                if (item3->kind == NODE_KIND_PROPERTY)
                                {
                                    node_property_t *property1 = (node_property_t *)item3->value;
                                    if (semantic_idcmp(property1->key, generic3->key) == 1)
                                    {
                                        semantic_error(program, property1->key, "already defined, previous in (%lld:%lld)",
                                            generic3->key->position.line, generic3->key->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (func2->parameters != NULL)
            {
                node_t *node3 = func2->parameters;
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
                                if (item4->kind == NODE_KIND_PROPERTY)
                                {
                                    node_property_t *property1 = (node_property_t *)item4->value;
                                    if (semantic_idcmp(property1->key, parameter3->key) == 1)
                                    {
                                        semantic_error(program, property1->key, "already defined, previous in (%lld:%lld)",
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
semantic_statement(program_t *program, node_t *node)
{
    if (node->kind == NODE_KIND_IF)
    {
        int32_t result;
        result = semantic_if(program, node);
        if (result == -1)
        {
            return -1;
        }
    }
    else 
    if (node->kind == NODE_KIND_FOR)
    {
        int32_t result;
        result = semantic_for(program, node);
        if (result == -1)
        {
            return -1;
        }
    }
    else
    if (node->kind == NODE_KIND_FORIN)
    {
        int32_t result;
        result = semantic_forin(program, node);
        if (result == -1)
        {
            return -1;
        }
    }
    else
    if (node->kind == NODE_KIND_TRY)
    {
        int32_t result;
        result = semantic_try(program, node);
        if (result == -1)
        {
            return -1;
        }
    }
    else
    if (node->kind == NODE_KIND_VAR)
    {
        int32_t result;
        result = semantic_var(program, node);
        if (result == -1)
        {
            return -1;
        }
    }
    return 1;
}

static int32_t
semantic_body(program_t *program, node_t *node)
{
    node_block_t *block1 = (node_block_t *)node->value;
    ilist_t *a1;
    for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
    {
        node_t *item = (node_t *)a1->value;
        int32_t result = semantic_statement(program, item);
        if (result == -1)
        {
            return -1;
        }
    }

    return 1;
}

static int32_t
semantic_func(program_t *program, node_t *node)
{
	node_fun_t *func1 = (node_fun_t *)node->value;

    node_t *sub = node;
    node_t *current = node->parent;
    while (current != NULL)
    {
        if (current->kind == NODE_KIND_MODULE)
        {
            node_module_t *module2 = (node_module_t *)current->value;

            ilist_t *a2;
            for (a2 = module2->items->begin;a2 != module2->items->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->id == sub->id)
                {
                    break;
                }

                if (item2->kind == NODE_KIND_CLASS)
                {
                    node_class_t *class3 = (node_class_t *)item2->value;
                    if (semantic_idcmp(func1->key, class3->key) == 1)
                    {
                        node_t *ngs1 = func1->generics;
                        node_t *ngs2 = class3->generics;
                        int32_t r1 = semantic_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *node3 = class3->block;
                            node_block_t *block3 = (node_block_t *)node3->value;
                            ilist_t *b3;
                            for (b3 = block3->list->begin;b3 != block3->list->end;b3 = b3->next)
                            {
                                node_t *item3 = (node_t *)b3->value;
                                if (item3->kind == NODE_KIND_FUN)
                                {
                                    node_fun_t *func3 = (node_fun_t *)item3->value;
                                    if (semantic_idstrcmp(func3->key, "Constructor") == 1)
                                    {
                                        node_t *nps1 = func1->parameters;
                                        node_t *nps2 = func3->parameters;
                                        int32_t r2 = semantic_eqaul_psps(program, nps1, nps2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            semantic_error(program, func1->key, "already defined, previous in (%lld:%lld)",
                                                class3->key->position.line, class3->key->position.column);
                                            return -1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_ENUM)
                {
                    node_enum_t *enum2 = (node_enum_t *)item2->value;

                    if (semantic_idcmp(func1->key, enum2->key) == 1)
                    {
                        node_t *ngs1 = func1->generics;
                        node_t *ngs2 = NULL;
                        int32_t r1 = semantic_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            semantic_error(program, func1->key, "already defined, previous in (%lld:%lld)",
                                enum2->key->position.line, enum2->key->position.column);
                            return -1;
                        }
                    }
                }
            }
            break;
        }
        else
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
                        if (semantic_idcmp(func1->key, generic1->key) == 1)
                        {
                            node_t *ngs1 = func1->generics;
                            node_t *ngs2 = NULL;
                            int32_t r1 = semantic_eqaul_gsgs(program, ngs1, ngs2);
                            if (r1 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r1 == 1)
                            {
                                semantic_error(program, func1->key, "already defined, previous in (%lld:%lld)",
                                    generic1->key->position.line, generic1->key->position.column);
                                return -1;
                            }
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

                        if (semantic_idcmp(func1->key, heritage1->key) == 1)
                        {
                            node_t *ngs1 = func1->generics;
                            node_t *ngs2 = NULL;
                            int32_t r1 = semantic_eqaul_gsgs(program, ngs1, ngs2);
                            if (r1 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r1 == 1)
                            {
                                semantic_error(program, func1->key, "already defined, previous in (%lld:%lld)",
                                    heritage1->key->position.line, heritage1->key->position.column);
                                return -1;
                            }
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

                if (item2->id == sub->id)
                {
                    break;
                }

                if (item2->kind == NODE_KIND_CLASS)
                {
                    node_class_t *class3 = (node_class_t *)item2->value;
                    if (semantic_idcmp(func1->key, class3->key) == 1)
                    {
                        node_t *ngs1 = func1->generics;
                        node_t *ngs2 = class3->generics;
                        int32_t r1 = semantic_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *node3 = class3->block;
                            node_block_t *block3 = (node_block_t *)node3->value;
                            ilist_t *b3;
                            for (b3 = block3->list->begin;b3 != block3->list->end;b3 = b3->next)
                            {
                                node_t *item3 = (node_t *)b3->value;
                                if (item3->kind == NODE_KIND_FUN)
                                {
                                    node_fun_t *func3 = (node_fun_t *)item3->value;
                                    if (semantic_idstrcmp(func3->key, "Constructor") == 1)
                                    {
                                        node_t *nps1 = func1->parameters;
                                        node_t *nps2 = func3->parameters;
                                        int32_t r2 = semantic_eqaul_psps(program, nps1, nps2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            semantic_error(program, func1->key, "already defined, previous in (%lld:%lld)",
                                                class3->key->position.line, class3->key->position.column);
                                            return -1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_ENUM)
                {
                    node_enum_t *enum2 = (node_enum_t *)item2->value;

                    if (semantic_idcmp(func1->key, enum2->key) == 1)
                    {
                        node_t *ngs1 = func1->generics;
                        node_t *ngs2 = NULL;
                        int32_t r1 = semantic_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            semantic_error(program, func1->key, "already defined, previous in (%lld:%lld)",
                                enum2->key->position.line, enum2->key->position.column);
                            return -1;
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_FUN)
                {
                    node_fun_t *func2 = (node_fun_t *)item2->value;

                    if (semantic_idcmp(func1->key, func2->key) == 1)
                    {
                        node_t *ngs1 = func1->generics;
                        node_t *ngs2 = func2->generics;
                        int32_t r1 = semantic_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *nps1 = func1->parameters;
                            node_t *nps2 = func2->parameters;
                            int32_t r2 = semantic_eqaul_psps(program, nps1, nps2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                semantic_error(program, func1->key, "already defined, previous in (%lld:%lld)",
                                    func2->key->position.line, func2->key->position.column);
                                return -1;
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_PROPERTY)
                {
                    node_property_t *property2 = (node_property_t *)item2->value;

                    if (semantic_idcmp(func1->key, property2->key) == 1)
                    {
                        node_t *ngs1 = func1->generics;
                        node_t *ngs2 = NULL;
                        int32_t r1 = semantic_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            semantic_error(program, func1->key, "already defined, previous in (%lld:%lld)",
                                property2->key->position.line, property2->key->position.column);
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

    if (func1->generics != NULL)
    {
        int32_t r1 = semantic_generics(program, func1->generics);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (func1->parameters != NULL)
    {
        int32_t r1 = semantic_parameters(program, func1->parameters);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (func1->body != NULL)
    {
        int32_t r1 = semantic_body(program, func1->body);
        if (r1 == -1)
        {
            return -1;
        }
    }

	return 1;
}

static int32_t
semantic_enum(program_t *program, node_t *node)
{
	node_enum_t *enum1 = (node_enum_t *)node->value;
    
    node_t *sub = node;
    node_t *current = node->parent;
    while (current != NULL)
    {
        if (current->kind == NODE_KIND_MODULE)
        {
            node_module_t *module2 = (node_module_t *)current->value;

            ilist_t *a2;
            for (a2 = module2->items->begin;a2 != module2->items->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->id == node->id)
                {
                    break;
                }

                if (item2->kind == NODE_KIND_IMPORT)
                {
                    node_import_t *import1 = (node_import_t *)item2->value;
                    node_t *node3 = import1->packages;
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
                                node_t *ngs1 = NULL;
                                node_t *ngs2 = package3->generics;
                                int32_t r1 = semantic_eqaul_gsgs(program, ngs1, ngs2);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 1)
                                {
                                    semantic_error(program, enum1->key, "already defined, previous in (%lld:%lld)",
                                        package3->key->position.line, package3->key->position.column);
                                    return -1;
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
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = class3->generics;
                        int32_t r1 = semantic_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            semantic_error(program, enum1->key, "already defined, previous in (%lld:%lld)",
                                class3->key->position.line, class3->key->position.column);
                            return -1;
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_ENUM)
                {
                    node_enum_t *enum2 = (node_enum_t *)item2->value;

                    if (semantic_idcmp(enum1->key, enum2->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = NULL;
                        int32_t r1 = semantic_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            semantic_error(program, enum1->key, "already defined, previous in (%lld:%lld)",
                                enum2->key->position.line, enum2->key->position.column);
                            return -1;
                        }
                    }
                }
            }
            break;
        }
        else
        if (current->kind == NODE_KIND_CLASS)
        {
            node_class_t *class2 = (node_class_t *)current->value;
            node_t *node2 = class2->block;
            node_block_t *block2 = (node_block_t *)node2->value;

            ilist_t *a2;
            for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;

                if (item2->id == sub->id)
                {
                    break;
                }

                if (item2->kind == NODE_KIND_CLASS)
                {
                    node_class_t *class3 = (node_class_t *)item2->value;
                    if (semantic_idcmp(enum1->key, class3->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = class3->generics;
                        int32_t r1 = semantic_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            semantic_error(program, enum1->key, "already defined, previous in (%lld:%lld)",
                                class3->key->position.line, class3->key->position.column);
                            return -1;
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_ENUM)
                {
                    node_enum_t *enum2 = (node_enum_t *)item2->value;

                    if (semantic_idcmp(enum1->key, enum2->key) == 1)
                    {
                        semantic_error(program, enum1->key, "already defined, previous in (%lld:%lld)",
                            enum2->key->position.line, enum2->key->position.column);
                        return -1;
                    }
                }
                else
                if (item2->kind == NODE_KIND_FUN)
                {
                    node_fun_t *func2 = (node_fun_t *)item2->value;

                    if (semantic_idcmp(enum1->key, func2->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = func2->generics;
                        int32_t r1 = semantic_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            semantic_error(program, enum1->key, "already defined, previous in (%lld:%lld)",
                                func2->key->position.line, func2->key->position.column);
                            return -1;
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_PROPERTY)
                {
                    node_property_t *property2 = (node_property_t *)item2->value;

                    if (semantic_idcmp(enum1->key, property2->key) == 1)
                    {
                        semantic_error(program, enum1->key, "already defined, previous in (%lld:%lld)",
                            property2->key->position.line, property2->key->position.column);
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
semantic_package(program_t *program, node_t *node)
{
    node_package_t *package1 = (node_package_t *)node->value;

    if (package1->route != NULL)
    {
        node_t *route1 = package1->route;
        while(route1 != NULL)
        {
            if (route1->kind == NODE_KIND_PSEUDONYM)
            {
                node_carrier_t *carrier1 = (node_carrier_t *)route1->value;
                route1 = carrier1->base;
            }
            else
            if (route1->kind == NODE_KIND_ATTRIBUTE)
            {
                node_binary_t *binary = (node_binary_t *)route1->value;
                route1 = binary->left;
            }
            else
            if (route1->kind == NODE_KIND_ID)
            {
                break;
            }
            else
            {
                semantic_error(program, package1->route, "not a route");
                return -1;
            }
        }  
    }

    node_t *sub = node;
    node_t *current = node->parent;
    while (current != NULL)
    {
        if (current->kind == NODE_KIND_PACKAGES)
        {
            node_block_t *block3 = (node_block_t *)current->value;

            ilist_t *a3;
            for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
            {
                node_t *item3 = (node_t *)a3->value;
                if (item3->kind == NODE_KIND_PACKAGE)
                {
                    if (item3->id == sub->id)
                    {
                        break;
                    }

                    node_package_t *package3 = (node_package_t *)item3->value;

                    if (semantic_idcmp(package1->key, package3->key) == 1)
                    {
                        node_t *ngs1 = package1->generics;
                        node_t *ngs2 = package3->generics;
                        int32_t r1 = semantic_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            semantic_error(program, package1->key, "already defined, previous in (%lld:%lld)",
                                package3->key->position.line, package3->key->position.column);
                            return -1;
                        }
                    }
                }
            }

            sub = current;
            current = current->parent;
            continue;
        }
        else
        if (current->kind == NODE_KIND_MODULE)
        {
            node_module_t *module2 = (node_module_t *)current->value;

            ilist_t *a2;
            for (a2 = module2->items->begin;a2 != module2->items->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                
                if (item2->id == sub->id)
                {
                    break;
                }

                if (item2->kind == NODE_KIND_IMPORT)
                {
                    node_import_t *import1 = (node_import_t *)item2->value;
                    node_t *node3 = import1->packages;
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
                                node_t *ngs1 = package1->generics;
                                node_t *ngs2 = package3->generics;
                                int32_t r1 = semantic_eqaul_gsgs(program, ngs1, ngs2);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 1)
                                {
                                    semantic_error(program, package1->key, "already defined, previous in (%lld:%lld)",
                                        package3->key->position.line, package3->key->position.column);
                                    return -1;
                                }
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_CLASS)
                {
                    node_class_t *class3 = (node_class_t *)item2->value;

                    if (semantic_idcmp(package1->key, class3->key) == 1)
                    {
                        node_t *ngs1 = package1->generics;
                        node_t *ngs2 = class3->generics;
                        int32_t r1 = semantic_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            semantic_error(program, package1->key, "already defined, previous in (%lld:%lld)",
                                class3->key->position.line, class3->key->position.column);
                            return -1;
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_ENUM)
                {
                    node_enum_t *enum2 = (node_enum_t *)item2->value;

                    if (semantic_idcmp(package1->key, enum2->key) == 1)
                    {
                        node_t *ngs1 = package1->generics;
                        node_t *ngs2 = NULL;
                        int32_t r1 = semantic_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            semantic_error(program, package1->key, "already defined, previous in (%lld:%lld)",
                                enum2->key->position.line, enum2->key->position.column);
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
semantic_packages(program_t *program, node_t *node)
{
    node_block_t *packages = (node_block_t *)node->value;
    
	ilist_t *a1;
    for (a1 = packages->list->begin;a1 != packages->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        int32_t result;
        result = semantic_package(program, item1);
        if (result == -1)
        {
            return -1;
        }
    }

	return 1;
}

static int32_t
semantic_import(program_t *program, node_t *node)
{
	node_import_t *import1 = (node_import_t *)node->value;

	if (import1->packages != NULL)
    {
        int32_t r1 = semantic_packages(program, import1->packages);
        if (r1 == -1)
        {
            return -1;
        }
    }

	return 1;
}

static int32_t
semantic_property(program_t *program, node_t *node)
{
    node_property_t *property1 = (node_property_t *)node->value;
    
    node_t *sub = node;
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
                            semantic_error(program, property1->key, "already defined, previous in (%lld:%lld)",
                                generic3->key->position.line, generic3->key->position.column);
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
                            semantic_error(program, property1->key, "already defined, previous in (%lld:%lld)",
                                heritage3->key->position.line, heritage3->key->position.column);
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
                
                if (item2->id == sub->id)
                {
                    break;
                }

                if (item2->kind == NODE_KIND_CLASS)
                {
                    node_class_t *class3 = (node_class_t *)item2->value;
                    if (semantic_idcmp(property1->key, class3->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = class3->generics;
                        int32_t r1 = semantic_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            semantic_error(program, property1->key, "already defined, previous in (%lld:%lld)",
                                class3->key->position.line, class3->key->position.column);
                            return -1;
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_FUN)
                {
                    node_fun_t *func2 = (node_fun_t *)item2->value;

                    if (semantic_idcmp(property1->key, func2->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = func2->generics;
                        int32_t r1 = semantic_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            semantic_error(program, property1->key, "already defined, previous in (%lld:%lld)",
                                func2->key->position.line, func2->key->position.column);
                            return -1;
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_ENUM)
                {
                    node_enum_t *enum2 = (node_enum_t *)item2->value;

                    if (semantic_idcmp(property1->key, enum2->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = NULL;
                        int32_t r1 = semantic_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            semantic_error(program, property1->key, "already defined, previous in (%lld:%lld)",
                                enum2->key->position.line, enum2->key->position.column);
                            return -1;
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_PROPERTY)
                {
                    node_property_t *property2 = (node_property_t *)item2->value;

                    if (semantic_idcmp(property1->key, property2->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = NULL;
                        int32_t r1 = semantic_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            semantic_error(program, property1->key, "already defined, previous in (%lld:%lld)",
                                property2->key->position.line, property2->key->position.column);
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
    
    list_t *response1 = list_create();
    if (response1 == NULL)
    {
        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    int32_t r1 = semantic_postfix(program, property1->type, response1);
    if (r1 == -1)
    {
        return -1;
    }
    else
    if (r1 == 0)
    {
        semantic_error(program, property1->type, "reference not found");
        return -1;
    }

    list_destroy(response1);

    return 1;
}

static int32_t
semantic_heritage(program_t *program, node_t *node)
{
    node_heritage_t *heritage1 = (node_heritage_t *)node->value;
    
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
                        semantic_error(program, heritage1->key, "already defined, previous in (%lld:%lld)",
                            heritage3->key->position.line, heritage3->key->position.column);
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
                            semantic_error(program, heritage1->key, "already defined, previous in (%lld:%lld)",
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
semantic_heritages(program_t *program, node_t *node)
{
	node_block_t *heritages = (node_block_t *)node->value;
    ilist_t *a1;
    for (a1 = heritages->list->begin;a1 != heritages->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        int32_t result;
        result = semantic_heritage(program, item1);
        if (result == -1)
        {
            return -1;
        }
    }
	return 1;
}

static int32_t
semantic_class(program_t *program, node_t *node)
{
	node_class_t *class1 = (node_class_t *)node->value;

    node_t *sub = node;
    node_t *current = node->parent;
    while (current != NULL)
    {
        if (current->kind == NODE_KIND_MODULE)
        {
            node_module_t *module2 = (node_module_t *)current->value;

            ilist_t *a2;
            for (a2 = module2->items->begin;a2 != module2->items->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->id == sub->id)
                {
                    break;
                }
                
                if (item2->kind == NODE_KIND_IMPORT)
                {
                    node_import_t *import1 = (node_import_t *)item2->value;
                    node_t *node3 = import1->packages;
                    node_block_t *block3 = (node_block_t *)node3->value;
                    ilist_t *a3;
                    for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                    {
                        node_t *item3 = (node_t *)a3->value;
                        if (item3->kind == NODE_KIND_PACKAGE)
                        {
                            node_package_t *package1 = (node_package_t *)item3->value;
                            if (semantic_idcmp(package1->key, class1->key) == 1)
                            {
                                node_t *ngs1 = NULL;
                                node_t *ngs2 = package1->generics;
                                int32_t r1 = semantic_eqaul_gsgs(program, ngs1, ngs2);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 1)
                                {
                                    semantic_error(program, class1->key, "already defined, previous in (%lld:%lld)",
                                        package1->key->position.line, package1->key->position.column);
                                    return -1;
                                }
                            }
                        }
                    }
                }
                else
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
                            node_t *node1 = class1->block;
                            node_block_t *block1 = (node_block_t *)node1->value;
    
                            ilist_t *b1;
                            for (b1 = block1->list->begin;b1 != block1->list->end;b1 = b1->next)
                            {
                                node_t *item1 = (node_t *)b1->value;
                                if (item1->kind == NODE_KIND_FUN)
                                {
                                    node_fun_t *func1 = (node_fun_t *)item1->value;
                                    if (semantic_idstrcmp(func1->key, "Constructor") == 1)
                                    {
                                        node_t *node3 = class3->block;
                                        node_block_t *block3 = (node_block_t *)node3->value;
                                        ilist_t *b3;
                                        for (b3 = block3->list->begin;b3 != block3->list->end;b3 = b3->next)
                                        {
                                            node_t *item3 = (node_t *)b3->value;
                                            if (item3->kind == NODE_KIND_FUN)
                                            {
                                                node_fun_t *func3 = (node_fun_t *)item3->value;
                                                if (semantic_idstrcmp(func3->key, "Constructor") == 1)
                                                {
                                                    node_t *nps1 = func1->parameters;
                                                    node_t *nps2 = func3->parameters;
                                                    int32_t r2 = semantic_eqaul_psps(program, nps1, nps2);
                                                    if (r2 == -1)
                                                    {
                                                        return -1;
                                                    }
                                                    else
                                                    if (r2 == 1)
                                                    {
                                                        semantic_error(program, class1->key, "already defined, previous in (%lld:%lld)",
                                                            class3->key->position.line, class3->key->position.column);
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
                if (item2->kind == NODE_KIND_ENUM)
                {
                    node_enum_t *enum2 = (node_enum_t *)item2->value;

                    if (semantic_idcmp(class1->key, enum2->key) == 1)
                    {
                        node_t *ngs1 = class1->generics;
                        node_t *ngs2 = NULL;
                        int32_t r1 = semantic_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            semantic_error(program, class1->key, "already defined, previous in (%lld:%lld)",
                                enum2->key->position.line, enum2->key->position.column);
                            return -1;
                        }
                    }
                }
            }
            break;
        }
        else
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
                        if (semantic_idcmp(class1->key, generic1->key) == 1)
                        {
                            node_t *ngs1 = class2->generics;
                            node_t *ngs2 = NULL;
                            int32_t r1 = semantic_eqaul_gsgs(program, ngs1, ngs2);
                            if (r1 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r1 == 1)
                            {
                                semantic_error(program, class1->key, "already defined, previous in (%lld:%lld)",
                                    generic1->key->position.line, generic1->key->position.column);
                                return -1;
                            }
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
                            node_t *ngs1 = class2->generics;
                            node_t *ngs2 = NULL;
                            int32_t r1 = semantic_eqaul_gsgs(program, ngs1, ngs2);
                            if (r1 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r1 == 1)
                            {
                                semantic_error(program, class1->key, "already defined, previous in (%lld:%lld)",
                                    heritage1->key->position.line, heritage1->key->position.column);
                                return -1;
                            }
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

                if (item2->id == sub->id)
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
                            node_t *node1 = class1->block;
                            node_block_t *block1 = (node_block_t *)node1->value;
    
                            ilist_t *b1;
                            for (b1 = block1->list->begin;b1 != block1->list->end;b1 = b1->next)
                            {
                                node_t *item1 = (node_t *)b1->value;
                                if (item1->kind == NODE_KIND_FUN)
                                {
                                    node_fun_t *func1 = (node_fun_t *)item1->value;
                                    if (semantic_idstrcmp(func1->key, "Constructor") == 1)
                                    {
                                        node_t *node3 = class3->block;
                                        node_block_t *block3 = (node_block_t *)node3->value;
                                        ilist_t *b3;
                                        for (b3 = block3->list->begin;b3 != block3->list->end;b3 = b3->next)
                                        {
                                            node_t *item3 = (node_t *)b3->value;
                                            if (item3->kind == NODE_KIND_FUN)
                                            {
                                                node_fun_t *func3 = (node_fun_t *)item3->value;
                                                if (semantic_idstrcmp(func3->key, "Constructor") == 1)
                                                {
                                                    node_t *nps1 = func1->parameters;
                                                    node_t *nps2 = func3->parameters;
                                                    int32_t r2 = semantic_eqaul_psps(program, nps1, nps2);
                                                    if (r2 == -1)
                                                    {
                                                        return -1;
                                                    }
                                                    else
                                                    if (r2 == 1)
                                                    {
                                                        semantic_error(program, class1->key, "already defined, previous in (%lld:%lld)",
                                                            class3->key->position.line, class3->key->position.column);
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
                if (item2->kind == NODE_KIND_FUN)
                {
                    node_fun_t *func2 = (node_fun_t *)item2->value;

                    if (semantic_idcmp(class1->key, func2->key) == 1)
                    {
                        node_t *ngs1 = class1->generics;
                        node_t *ngs2 = func2->generics;
                        int32_t r1 = semantic_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *node1 = class1->block;
                            node_block_t *block1 = (node_block_t *)node1->value;

                            ilist_t *b1;
                            for (b1 = block1->list->begin;b1 != block1->list->end;b1 = b1->next)
                            {
                                node_t *item1 = (node_t *)b1->value;
                                if (item1->kind == NODE_KIND_FUN)
                                {
                                    node_fun_t *func1 = (node_fun_t *)item1->value;
                                    if (semantic_idstrcmp(func1->key, "Constructor") == 1)
                                    {
                                        node_t *nps1 = func1->parameters;
                                        node_t *nps2 = func2->parameters;
                                        int32_t r2 = semantic_eqaul_psps(program, nps1, nps2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            semantic_error(program, class1->key, "already defined, previous in (%lld:%lld)",
                                                func2->key->position.line, func2->key->position.column);
                                            return -1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_ENUM)
                {
                    node_enum_t *enum2 = (node_enum_t *)item2->value;

                    if (semantic_idcmp(class1->key, enum2->key) == 1)
                    {
                        node_t *ngs1 = class1->generics;
                        node_t *ngs2 = NULL;
                        int32_t r1 = semantic_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            semantic_error(program, class1->key, "already defined, previous in (%lld:%lld)",
                                enum2->key->position.line, enum2->key->position.column);
                            return -1;
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_PROPERTY)
                {
                    node_property_t *property2 = (node_property_t *)item2->value;

                    if (semantic_idcmp(class1->key, property2->key) == 1)
                    {
                        node_t *ngs1 = class1->generics;
                        node_t *ngs2 = NULL;
                        int32_t r1 = semantic_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            semantic_error(program, class1->key, "already defined, previous in (%lld:%lld)",
                                property2->key->position.line, property2->key->position.column);
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

    if (class1->generics != NULL)
    {
        int32_t r1 = semantic_generics(program, class1->generics);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (class1->heritages != NULL)
    {
        int32_t r1 = semantic_heritages(program, class1->heritages);
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
        node_t *item = (node_t *)a1->value;
        
        if (item->kind == NODE_KIND_CLASS)
        {
            int32_t result;
            result = semantic_class(program, item);
            if (result == -1)
            {
                return -1;
            }
        }
        else
        if (item->kind == NODE_KIND_ENUM)
        {
            int32_t result;
            result = semantic_enum(program, item);
            if (result == -1)
            {
                return -1;
            }
        }
        else
        if (item->kind == NODE_KIND_FUN)
        {
            int32_t result;
            result = semantic_func(program, item);
            if (result == -1)
            {
                return -1;
            }
        }
        else
        if (item->kind == NODE_KIND_PROPERTY)
        {
            int32_t result;
            result = semantic_property(program, item);
            if (result == -1)
            {
                return -1;
            }
        }
    }

	return 1;
}

static int32_t
semantic_module(program_t *program, node_t *node)
{
	node_module_t *module = (node_module_t *)node->value;

    ilist_t *a1;
    for (a1 = module->items->begin; a1 != module->items->end; a1 = a1->next)
    {
        node_t *item = (node_t *)a1->value;

        if (item->kind == NODE_KIND_IMPORT)
        {
            int32_t result;
            result = semantic_import(program, item);
            if (result == -1)
            {
                return -1;
            }
        }
        else
        if (item->kind == NODE_KIND_CLASS)
        {
            int32_t result;
            result = semantic_class(program, item);
            if (result == -1)
            {
                return -1;
            }
        }
        else
        if (item->kind == NODE_KIND_ENUM)
        {
            int32_t result;
            result = semantic_enum(program, item);
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
	int32_t result = semantic_module(program, node);
	if(result == -1)
	{
		return -1;
	}
	return 1;
}