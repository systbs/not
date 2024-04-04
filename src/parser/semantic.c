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


typedef enum select_flag 
{
    SELECT_FLAG_NONE    = 0,
    SELECT_FLAG_FOLLOW  = 1 << 0, // follow is shows the method reversible or not
    SELECT_FLAG_OBJECT  = 1 << 1, // its work as search for none-types
} select_flag_t;

static int32_t
semantic_postfix(program_t *program, node_t *scope, node_t *node, list_t *response, uint64_t flag);

static int32_t
semantic_resolve(program_t *program, node_t *scope, node_t *node, list_t *response, uint64_t flag);

static int32_t
semantic_expression(program_t *program, node_t *scope, node_t *node, list_t *response, uint64_t flag);


static int32_t
semantic_parameters(program_t *program, node_t *node);

static int32_t
semantic_generics(program_t *program, node_t *node);

static int32_t
semantic_body(program_t *program, node_t *node);

static int32_t
semantic_fields(program_t *program, node_t *node);


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
                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r2 = semantic_postfix(program, NULL, heritage2->type, response2, SELECT_FLAG_NONE);
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
                                    
                                    int32_t r1 = semantic_postfix(program, NULL, generic1->type, response1, SELECT_FLAG_NONE);
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
                                            
                                            int32_t r2 = semantic_postfix(program, NULL, generic2->type, response2, SELECT_FLAG_NONE);
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

                                        int32_t r1 = semantic_postfix(program, NULL, generic1->type, response1, SELECT_FLAG_NONE);
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
                                                
                                                int32_t r2 = semantic_postfix(program, NULL, generic2->type, response2, SELECT_FLAG_NONE);
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

                                        int32_t r1 = semantic_postfix(program, NULL, generic1->type, response1, SELECT_FLAG_NONE);
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
                                                
                                                int32_t r2 = semantic_postfix(program, NULL, generic2->type, response2, SELECT_FLAG_NONE);
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
                            
                            int32_t r1 = semantic_postfix(program, NULL, parameter1->type, response1, SELECT_FLAG_NONE);
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
                                    
                                    int32_t r2 = semantic_postfix(program, NULL, parameter2->type, response2, SELECT_FLAG_NONE);
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
                                
                                int32_t r1 = semantic_postfix(program, NULL, parameter1->type, response1, SELECT_FLAG_NONE);
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
                                        
                                        int32_t r2 = semantic_postfix(program, NULL, parameter2->type, response2, SELECT_FLAG_NONE);
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
                                
                                int32_t r1 = semantic_postfix(program, NULL, parameter1->type, response1, SELECT_FLAG_NONE);
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
                                        
                                        int32_t r2 = semantic_postfix(program, NULL, parameter2->type, response2, SELECT_FLAG_NONE);
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
            uint64_t cnt2 = 0;
			int32_t use_by_key = 0;

			list_t *repository1 = list_create();
			if (repository1 == NULL)
			{
				fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
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
									fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
									return -1;
								}

								if (parameter1->type != NULL)
								{
									list_t *response1 = list_create();
									if (response1 == NULL)
									{
										fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
										return -1;
									}
									
									int32_t r1 = semantic_postfix(program, NULL, parameter1->type, response1, SELECT_FLAG_NONE);
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
											
											int32_t r2 = semantic_expression(program, NULL, argument2->key, response2, SELECT_FLAG_NONE);
											if (r2 == -1)
											{
												return -1;
											}
											else
											if (r2 == 0)
											{
												semantic_error(program, argument2->key, "reference not found");
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
										fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
										return -1;
									}

									if (parameter1->type != NULL)
									{
										list_t *response1 = list_create();
										if (response1 == NULL)
										{
											fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
											return -1;
										}
										
										int32_t r1 = semantic_postfix(program, NULL, parameter1->type, response1, SELECT_FLAG_NONE);
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
												
												int32_t r2 = semantic_expression(program, NULL, argument2->value, response2, SELECT_FLAG_NONE);
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
				fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
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
									fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
									return -1;
								}

								if (generic1->type != NULL)
								{
									list_t *response1 = list_create();
									if (response1 == NULL)
									{
										fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
										return -1;
									}
									
									int32_t r1 = semantic_postfix(program, NULL, generic1->type, response1, SELECT_FLAG_NONE);
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
											
											int32_t r2 = semantic_postfix(program, NULL, field2->key, response2, SELECT_FLAG_NONE);
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
										fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
										return -1;
									}

									if (generic1->type != NULL)
									{
										list_t *response1 = list_create();
										if (response1 == NULL)
										{
											fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
											return -1;
										}
										
										int32_t r1 = semantic_postfix(program, NULL, generic1->type, response1, SELECT_FLAG_NONE);
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
												
												int32_t r2 = semantic_postfix(program, NULL, field2->value, response2, SELECT_FLAG_NONE);
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


static int32_t
semantic_substitute_gsfs(program_t *program, node_t *ngs1, node_t *nfs2)
{
    if (ngs1 == NULL)
    {
        return 1;
    }
    else
    {
        if (nfs2 == NULL)
        {
            return 1;
        }
        uint64_t cnt2 = 0;
        node_block_t *bfs2 = (node_block_t *)nfs2->value;
        ilist_t *a2;
        for (a2 = bfs2->list->begin;a2 != bfs2->list->end;a2 = a2->next)
        {
            node_t *item2 = (node_t *)a2->value;
            if (item2->kind == NODE_KIND_FIELD)
            {
                cnt2 += 1;
                node_field_t *field2 = (node_field_t *)item2->value;
                if (field2->value == NULL)
                {
                    uint64_t cnt1 = 0;
                    node_block_t *bgs1 = (node_block_t *)ngs1->value;
                    ilist_t *a1;
                    for (a1 = bgs1->list->begin;a1 != bgs1->list->end;a1 = a1->next)
                    {
                        node_t *item1 = (node_t *)a1->value;
                        if (item1->kind == NODE_KIND_GENERIC)
                        {
                            node_generic_t *generic1 = (node_generic_t *)item1->value;
                            cnt1 += 1;
                            if (cnt1 < cnt2)
                            {
                                continue;
                            }
                            list_t *response1 = list_create();
                            if (response1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }
                            
                            int32_t r1 = semantic_postfix(program, NULL, field2->key, response1, SELECT_FLAG_NONE);
                            if (r1 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r1 == 0)
                            {
                                semantic_error(program, field2->key, "reference not found");
                                return -1;
                            }
                            else
                            if (r1 == 1)
                            {
                                ilist_t *a3;
                                for (a3 = response1->begin;a3 != response1->end;a3 = a3->next)
                                {
                                    node_t *item3 = (node_t *)a3->value;
                                    if (item3->kind == NODE_KIND_GENERIC)
                                    {
                                        node_generic_t *generic3 = (node_generic_t *)item3->value;
                                        if (generic3->value_update != NULL)
                                        {
                                            generic1->value_update = generic3->value_update;
                                        }
                                    }
                                    else
                                    {
                                        generic1->value_update = item3;
                                    }
                                }
                            }
                            list_destroy(response1);
                            break;
                        }
                    }
                }
                else
                {
                    node_block_t *bgs1 = (node_block_t *)ngs1->value;
                    ilist_t *a1;
                    for (a1 = bgs1->list->begin;a1 != bgs1->list->end;a1 = a1->next)
                    {
                        node_t *item1 = (node_t *)a1->value;
                        if (item1->kind == NODE_KIND_GENERIC)
                        {
                            node_generic_t *generic1 = (node_generic_t *)item1->value;
                            if (semantic_idcmp(generic1->key, field2->key) == 1)
                            {
                                list_t *response1 = list_create();
                                if (response1 == NULL)
                                {
                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                
                                int32_t r1 = semantic_postfix(program, NULL, field2->value, response1, SELECT_FLAG_NONE);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 0)
                                {
                                    semantic_error(program, field2->value, "reference not found");
                                    return -1;
                                }
                                else
                                if (r1 == 1)
                                {
                                    ilist_t *a3;
                                    for (a3 = response1->begin;a3 != response1->end;a3 = a3->next)
                                    {
                                        node_t *item3 = (node_t *)a3->value;
                                        if (item3->kind == NODE_KIND_GENERIC)
                                        {
                                            node_generic_t *generic3 = (node_generic_t *)item3->value;
                                            if (generic3->value_update != NULL)
                                            {
                                                generic1->value_update = generic3->value_update;
                                            }
                                        }
                                        else
                                        {
                                            generic1->value_update = item3;
                                        }
                                    }
                                }
                                list_destroy(response1);
                                break;
                            }
                        }
                    }
                }
            }
        }

        return 1;
    }
}

static int32_t
semantic_substitute_psas(program_t *program, node_t *nps1, node_t *nas2)
{
	uint64_t cnt2 = 0;
	node_block_t *bas2 = (node_block_t *)nas2->value;
	ilist_t *a2;
	for (a2 = bas2->list->begin;a2 != bas2->list->end;a2 = a2->next)
	{
		node_t *item2 = (node_t *)a2->value;
		if (item2->kind == NODE_KIND_FIELD)
		{
			cnt2 += 1;
			node_argument_t *argument2 = (node_argument_t *)item2->value;
			if (argument2->value == NULL)
			{
				uint64_t cnt1 = 0;
				node_block_t *bps1 = (node_block_t *)nps1->value;
				ilist_t *a1;
				for (a1 = bps1->list->begin;a1 != bps1->list->end;a1 = a1->next)
				{
					node_t *item1 = (node_t *)a1->value;
					if (item1->kind == NODE_KIND_PARAMETER)
					{
						node_parameter_t *parameter1 = (node_parameter_t *)item1->value;
						cnt1 += 1;
						if (cnt1 < cnt2)
						{
							continue;
						}

						parameter1->value_update = argument2->value;
						break;
					}
				}
			}
			else
			{
				node_block_t *bps1 = (node_block_t *)nps1->value;
				ilist_t *a1;
				for (a1 = bps1->list->begin;a1 != bps1->list->end;a1 = a1->next)
				{
					node_t *item1 = (node_t *)a1->value;
					if (item1->kind == NODE_KIND_PARAMETER)
					{
						node_parameter_t *parameter1 = (node_parameter_t *)item1->value;
						if (semantic_idcmp(parameter1->key, argument2->key) == 1)
						{
							parameter1->value_update = argument2->value;
							break;
						}
					}
				}
			}
			
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




/*
 * @Parameters: 
 *		root is where search node by name
 *		scope is used to store the last scope of the deparcated class/module
 *		name is the name of the node we are looking for
 * @Return: 1 if found, 0 if not found, -1 if wrong
*/
static int32_t
semantic_select(program_t *program, node_t *root, node_t *scope, node_t *name, list_t *response, uint64_t flag)
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
                            return 1;
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
                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
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
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
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
                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
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
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
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
                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
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
                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
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
                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
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
                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
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
                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
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
                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
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
                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
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
                    node_t *clone1 = node_clone(item1->parent, item1);
                    if (clone1 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    if ((flag & SELECT_FLAG_OBJECT) == SELECT_FLAG_OBJECT)
                    {
                        if ((clone1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                        {
                            if ((class2->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                            {
                                continue;
                                /* semantic_error(program, clone1, "non static, requested for (%lld:%lld)", 
                                        name->position.line, name->position.column);
                                return -1; */
                            }
                        }
                    }

                    if (scope != NULL)
                    {
                        clone1->flag |= NODE_FLAG_DERIVE;

                        if ((class2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                        {
                            continue;
                            /* semantic_error(program, clone1, "private access, requested for (%lld:%lld)", 
                                    name->position.line, name->position.column);
                            return -1; */
                        }

                        if ((flag & SELECT_FLAG_FOLLOW) != SELECT_FLAG_FOLLOW)
                        {
                            if ((class2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                            {
                                continue;
                                /* semantic_error(program, clone1, "protect access, requested for (%lld:%lld)", 
                                    name->position.line, name->position.column);
                                return -1; */
                            }
                        }
                    }
                    else
                    {
                        clone1->flag &= ~NODE_FLAG_DERIVE;
                    }

                    ilist_t *r1 = list_rpush(response, clone1);
                    if (r1 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                    //return 1;
                }
            }
            else
            if (item1->kind == NODE_KIND_ENUM)
            {
                node_enum_t *enum1 = (node_enum_t *)item1->value;
                if (semantic_idcmp(enum1->key, name) == 1)
                {
                    if ((flag & SELECT_FLAG_OBJECT) == SELECT_FLAG_OBJECT)
                    {
                        if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                        {
                            if ((enum1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                            {
                                continue;
                                /* semantic_error(program, item1, "non static, requested for (%lld:%lld)", 
                                        name->position.line, name->position.column);
                                return -1; */
                            }
                        }
                    }

                    if (scope != NULL)
                    {
                        item1->flag |= NODE_FLAG_DERIVE;

                        if ((enum1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                        {
                            continue;
                            /* semantic_error(program, item1, "private access, requested for (%lld:%lld)", 
                                    name->position.line, name->position.column);
                            return -1; */
                        }

                        if ((flag & SELECT_FLAG_FOLLOW) != SELECT_FLAG_FOLLOW)
                        {
                            if ((enum1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                            {
                                continue;
                                /* semantic_error(program, item1, "protect access, requested for (%lld:%lld)", 
                                    name->position.line, name->position.column);
                                return -1; */
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
                    return 1;
                }
            }
            else
            if (item1->kind == NODE_KIND_FUN)
            {
                node_fun_t *fun1 = (node_fun_t *)item1->value;
                if (semantic_idcmp(fun1->key, name) == 1)
                {
                    node_t *clone1 = node_clone(item1->parent, item1);
                    if (clone1 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    if ((clone1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                    {
                        if ((fun1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                        {
                            continue;
                            /* semantic_error(program, clone1, "non static, requested for (%lld:%lld)", 
                                    name->position.line, name->position.column);
                            return -1; */
                        }
                    }

                    if (scope != NULL)
                    {
                        clone1->flag |= NODE_FLAG_DERIVE;

                        if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                        {
                            continue;
                            /* semantic_error(program, clone1, "private access, requested for (%lld:%lld)", 
                                    name->position.line, name->position.column);
                            return -1; */
                        }

                        if ((flag & SELECT_FLAG_FOLLOW) != SELECT_FLAG_FOLLOW)
                        {
                            if ((fun1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                            {
                                continue;
                                /* semantic_error(program, clone1, "protect access, requested for (%lld:%lld)", 
                                    name->position.line, name->position.column);
                                return -1; */
                            }
                        }
                    }
                    else
                    {
                        clone1->flag &= ~NODE_FLAG_DERIVE;
                    }

                    ilist_t *r1 = list_rpush(response, clone1);
                    if (r1 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                    //return 1;
                }
            }
            else
            if (item1->kind == NODE_KIND_PROPERTY)
            {
                node_property_t *property1 = (node_property_t *)item1->value;

                if (semantic_idcmp(property1->key, name) == 1)
                {
                    if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                    {
                        if ((property1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                        {
                            continue;
                            /* semantic_error(program, item1, "non static, requested for (%lld:%lld)", 
                                    name->position.line, name->position.column);
                            return -1; */
                        }
                    }

                    if (scope != NULL)
                    {
                        item1->flag |= NODE_FLAG_DERIVE;

                        if ((property1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                        {
                            continue;
                            /* semantic_error(program, item1, "private access, requested for (%lld:%lld)", 
                                    name->position.line, name->position.column);
                            return -1; */
                        }

                        if ((flag & SELECT_FLAG_FOLLOW) != SELECT_FLAG_FOLLOW)
                        {
                            if ((property1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                            {
                                continue;
                                /* semantic_error(program, item1, "protect access, requested for (%lld:%lld)", 
                                    name->position.line, name->position.column);
                                return -1; */
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
                    return 1;
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
                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }
                        return 1;
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
                        
                        int32_t r1 = semantic_postfix(program, NULL, heritage3->type, response2, SELECT_FLAG_NONE);
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
                                    int32_t r2 = semantic_select(program, item3, NULL, name, response, flag | SELECT_FLAG_FOLLOW);
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
                                if (item3->kind == NODE_KIND_PACKAGE)
                                {
                                    node_t *root1 = item3->parent;
                                    while (root1 != NULL)
                                    {
                                        if (root1->kind == NODE_KIND_IMPORT)
                                        {
                                            break;
                                        }
                                        else
                                        {
                                            root1 = root1->parent;
                                        }
                                    }

                                    if (root1->kind != NODE_KIND_IMPORT)
                                    {
                                        semantic_error(program, root1, "import not found\n");
                                        return -1;
                                    }

                                    node_import_t *import1 = (node_import_t *)root1->value;

                                    node_basic_t *basic2 = (node_basic_t *)import1->path->value;
                                    
                                    node_t *module1 = program_load(program, basic2->value);
                                    if (module1 == NULL)
                                    {
                                        return -1;
                                    }

                                    node_package_t *package1 = (node_package_t *)item3->value;

                                    list_t *response3 = list_create();
                                    if (response3 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_postfix(program, module1, package1->route, response3, SELECT_FLAG_NONE);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, package1->route, "reference not found");
                                        return -1;
                                    }
                                    else
                                    if (r2 == 1)
                                    {
                                        ilist_t *a4;
                                        for (a4 = response3->begin;a4 != response3->end;a4 = a4->next)
                                        {
                                            node_t *item4 = (node_t *)a4->value;
                                            if (item4->kind == NODE_KIND_CLASS)
                                            {
                                                heritage3->value_update = item4;
                                                int32_t r2 = semantic_select(program, item4, NULL, name, response, flag | SELECT_FLAG_FOLLOW);
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
                                            {
                                                semantic_error(program, item4, "non-class as heritage, for (%lld:%lld)",
                                                    heritage3->type->position.line, heritage3->type->position.column);
                                                return -1;
                                            }
                                        }
                                    }
                                    list_destroy(response3);
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
                        node_t *item3 = heritage3->value_update;
                        if (item3->kind == NODE_KIND_CLASS)
                        {
                            int32_t r2 = semantic_select(program, heritage3->value_update, NULL, name, response, flag | SELECT_FLAG_FOLLOW);
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
                            node_t *clone1 = node_clone(item2->parent, item2);
                            if (clone1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

							if (scope != NULL)
                    		{
								clone1->flag |= NODE_FLAG_DERIVE;
							}
							else
							{
								clone1->flag &= ~NODE_FLAG_DERIVE;
							}

                            ilist_t *r1 = list_rpush(response, clone1);
                            if (r1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }
                            //return 1;
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
                    node_t *clone1 = node_clone(item1->parent, item1);
                    if (clone1 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                    class2 = (node_class_t *)clone1->value;

                    if ((flag & SELECT_FLAG_OBJECT) == SELECT_FLAG_OBJECT)
                    {
                        if ((clone1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                        {
                            if ((class2->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                            {
                                continue;
                                /*
                                semantic_error(program, clone1, "non static, requested for (%lld:%lld)", 
                                        name->position.line, name->position.column);
                                return -1;
                                */
                            }
                        }
                    }

                    if (scope != NULL)
                    {
                        clone1->flag |= NODE_FLAG_DERIVE;

						if ((class2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                        {
                            continue;
                            /*
                            semantic_error(program, clone1, "private access, requested for (%lld:%lld)", 
                                    name->position.line, name->position.column);
                            return -1;
                            */
                        }

                        if ((flag & SELECT_FLAG_FOLLOW) != SELECT_FLAG_FOLLOW)
                        {
                            if ((class2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                            {
                                continue;
                                /*
                                semantic_error(program, clone1, "protect access, requested for (%lld:%lld)", 
                                    name->position.line, name->position.column);
                                return -1;
                                */
                            }
                        }
                    }
                    else
                    {
                        clone1->flag &= ~NODE_FLAG_DERIVE;
                    }

                    ilist_t *r1 = list_rpush(response, clone1);
                    if (r1 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                    //return 1;
                }
            }
            else
            if (item1->kind == NODE_KIND_ENUM)
            {
                node_enum_t *enum1 = (node_enum_t *)item1->value;
                if (semantic_idcmp(enum1->key, name) == 1)
                {
                    if ((flag & SELECT_FLAG_OBJECT) == SELECT_FLAG_OBJECT)
                    {
                        if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                        {
                            if ((enum1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                            {
                                continue;
                                /* semantic_error(program, item1, "non static, requested for (%lld:%lld)", 
                                        name->position.line, name->position.column);
                                return -1; */
                            }
                        }
                    }

                    if (scope != NULL)
                    {
                        item1->flag |= NODE_FLAG_DERIVE;

						if ((enum1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                        {
                            continue;
                            /* semantic_error(program, item1, "private access, requested for (%lld:%lld)", 
                                    name->position.line, name->position.column);
                            return -1; */
                        }

                        if ((flag & SELECT_FLAG_FOLLOW) != SELECT_FLAG_FOLLOW)
                        {
                            if ((enum1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                            {
                                continue;
                                /* semantic_error(program, item1, "protect access, requested for (%lld:%lld)", 
                                    name->position.line, name->position.column);
                                return -1; */
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
                    return 1;
                }
            }
        }
    }
    
    if (root->parent != NULL && ((flag & SELECT_FLAG_FOLLOW) != SELECT_FLAG_FOLLOW))
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

    if (list_count(response) > 0)
    {
        return 1;
    }

    return 0;
}

/*
 * @Parameters:
 *      scope used for select as root
 *      node is current node
 * @Return: 1 if found, 0 if not found, -1 if wrong
*/
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
    node_basic_t *basic1 = (node_basic_t *)node->value;
    char *str = basic1->value;
    if (str[strlen(str)] == 'j')
    {
        // complex
        node_t *node2 = node_create(node, node->position);
        if (node2 == NULL)
        {
            return -1;
        }
        node_t *node3 = node_make_id(node2, "Complex32");
        if (!node3)
        {
            return -1;
        }
        node3->flag = NODE_FLAG_TEMPORARY;

        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_select(program, node3->parent, NULL, node3, response1, SELECT_FLAG_NONE);
		if (r1 == -1)
		{
			return -1;
		}
        if (r1 == 0)
        {
            semantic_error(program, node3->parent, "reference not found");
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
                    node_t *clone1 = node_clone(item1->parent, item1);
                    if (clone1 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                    clone1->flag |= NODE_FLAG_NEW;

                    ilist_t *il1 = list_rpush(response, clone1);
                    if (il1 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                    node_prug(node3);
                    list_destroy(response1);
                    return 1;
                }
                else
                if (item1->kind == NODE_KIND_PACKAGE)
                {
                    node_package_t *package1 = (node_package_t *)item1->value;

                    node_t *root1 = item1->parent;
                    while (root1 != NULL)
                    {
                        if (root1->kind == NODE_KIND_IMPORT)
                        {
                            break;
                        }
                        else
                        {
                            root1 = root1->parent;
                        }
                    }

                    if (root1->kind != NODE_KIND_IMPORT)
                    {
                        semantic_error(program, root1, "import not found\n");
                        return -1;
                    }

                    node_import_t *import1 = (node_import_t *)root1->value;

                    node_basic_t *basic2 = (node_basic_t *)import1->path->value;

                    node_t *module1 = program_load(program, basic2->value);
                    if (module1 == NULL)
                    {
                        return -1;
                    }

                    list_t *response2 = list_create();
                    if (response2 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r2 = semantic_postfix(program, module1, package1->route, response2, SELECT_FLAG_NONE);
                    if (r2 == -1)
                    {
                        return -1;
                    }
                    else
                    if (r2 == 0)
                    {
                        semantic_error(program, package1->route, "reference not found");
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
                                node_t *clone2 = node_clone(item2->parent, item2);
                                if (clone2 == NULL)
                                {
                                    return -1;
                                }
                                clone2->flag |= NODE_FLAG_NEW;

                                ilist_t *il1 = list_rpush(response, clone2);
                                if (il1 == NULL)
                                {
                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                            }
                            else
                            {
                                semantic_error(program, item2, "non-class");
                                return -1;
                            }
                            return 1;

                        }
                    }
                    list_destroy(response2);
                }
                else
                {
                    semantic_error(program, item1, "reference not found");
                    return -1;
                }
            }
        }
        node_prug(node3);
        list_destroy(response1);
        return 0;
    }
    else
    {
        double128_t value = utils_stold(str);
        if (utils_isinteger(value))
        {
            if ((value > MIN_INT32) && (value < MAX_INT32))
            {
                // int
                node_t *node2 = node_create(node, node->position);
                if (node2 == NULL)
                {
                    return -1;
                }
                node_t *node3 = node_make_id(node2, "Int32");
                if (!node3)
                {
                    return -1;
                }
                node3->flag = NODE_FLAG_TEMPORARY;

                list_t *response1 = list_create();
                if (response1 == NULL)
                {
                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                    return -1;
                }

                int32_t r1 = semantic_select(program, node3->parent, NULL, node3, response1, SELECT_FLAG_NONE);
                if (r1 == -1)
                {
                    return -1;
                }
                if (r1 == 0)
                {
                    semantic_error(program, node3->parent, "reference not found");
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
                            node_t *clone1 = node_clone(item1->parent, item1);
                            if (clone1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }
                            clone1->flag |= NODE_FLAG_NEW;

                            ilist_t *il1 = list_rpush(response, clone1);
                            if (il1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }
                            node_prug(node3);
                            list_destroy(response1);
                            return 1;
                        }
                        else
                        if (item1->kind == NODE_KIND_PACKAGE)
                        {
                            node_package_t *package1 = (node_package_t *)item1->value;

                            node_t *root1 = item1->parent;
                            while (root1 != NULL)
                            {
                                if (root1->kind == NODE_KIND_IMPORT)
                                {
                                    break;
                                }
                                else
                                {
                                    root1 = root1->parent;
                                }
                            }

                            if (root1->kind != NODE_KIND_IMPORT)
                            {
                                semantic_error(program, root1, "import not found\n");
                                return -1;
                            }

                            node_import_t *import1 = (node_import_t *)root1->value;

                            node_basic_t *basic2 = (node_basic_t *)import1->path->value;

                            node_t *module1 = program_load(program, basic2->value);
                            if (module1 == NULL)
                            {
                                return -1;
                            }

                            list_t *response2 = list_create();
                            if (response2 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r2 = semantic_postfix(program, module1, package1->route, response2, SELECT_FLAG_NONE);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 0)
                            {
                                semantic_error(program, package1->route, "reference not found");
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
                                        node_t *clone2 = node_clone(item2->parent, item2);
                                        if (clone2 == NULL)
                                        {
                                            return -1;
                                        }
                                        clone2->flag |= NODE_FLAG_NEW;

                                        ilist_t *il1 = list_rpush(response, clone2);
                                        if (il1 == NULL)
                                        {
                                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }
                                    else
                                    {
                                        semantic_error(program, item2, "non-class");
                                        return -1;
                                    }
                                    return 1;

                                }
                            }
                            list_destroy(response2);
                        }
                        else
                        {
                            semantic_error(program, item1, "reference not found");
                            return -1;
                        }
                    }
                }
                node_prug(node3);
                list_destroy(response1);
                return 0;
            }
            else
            {
                // long
                node_t *node2 = node_create(node, node->position);
                if (node2 == NULL)
                {
                    return -1;
                }
                node_t *node3 = node_make_id(node2, "Int64");
                if (!node3)
                {
                    return -1;
                }
                node3->flag = NODE_FLAG_TEMPORARY;

                list_t *response1 = list_create();
                if (response1 == NULL)
                {
                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                    return -1;
                }

                int32_t r1 = semantic_select(program, node3->parent, NULL, node3, response1, SELECT_FLAG_NONE);
                if (r1 == -1)
                {
                    return -1;
                }
                if (r1 == 0)
                {
                    semantic_error(program, node3->parent, "reference not found");
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
                            node_t *clone1 = node_clone(item1->parent, item1);
                            if (clone1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }
                            clone1->flag |= NODE_FLAG_NEW;

                            ilist_t *il1 = list_rpush(response, clone1);
                            if (il1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }
                            node_prug(node3);
                            list_destroy(response1);
                            return 1;
                        }
                        else
                        if (item1->kind == NODE_KIND_PACKAGE)
                        {
                            node_package_t *package1 = (node_package_t *)item1->value;

                            node_t *root1 = item1->parent;
                            while (root1 != NULL)
                            {
                                if (root1->kind == NODE_KIND_IMPORT)
                                {
                                    break;
                                }
                                else
                                {
                                    root1 = root1->parent;
                                }
                            }

                            if (root1->kind != NODE_KIND_IMPORT)
                            {
                                semantic_error(program, root1, "import not found\n");
                                return -1;
                            }

                            node_import_t *import1 = (node_import_t *)root1->value;

                            node_basic_t *basic2 = (node_basic_t *)import1->path->value;

                            node_t *module1 = program_load(program, basic2->value);
                            if (module1 == NULL)
                            {
                                return -1;
                            }

                            list_t *response2 = list_create();
                            if (response2 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r2 = semantic_postfix(program, module1, package1->route, response2, SELECT_FLAG_NONE);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 0)
                            {
                                semantic_error(program, package1->route, "reference not found");
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
                                        node_t *clone2 = node_clone(item2->parent, item2);
                                        if (clone2 == NULL)
                                        {
                                            return -1;
                                        }
                                        clone2->flag |= NODE_FLAG_NEW;

                                        ilist_t *il1 = list_rpush(response, clone2);
                                        if (il1 == NULL)
                                        {
                                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }
                                    else
                                    {
                                        semantic_error(program, item2, "non-class");
                                        return -1;
                                    }
                                    return 1;

                                }
                            }
                            list_destroy(response2);
                        }
                        else
                        {
                            semantic_error(program, item1, "reference not found");
                            return -1;
                        }
                    }
                }
                node_prug(node3);
                list_destroy(response1);
                return 0;
            }
        }
        else
        {
            if ((value > MIN_INT32) && (value < MAX_INT32))
            {
                // float
                node_t *node2 = node_create(node, node->position);
                if (node2 == NULL)
                {
                    return -1;
                }
                node_t *node3 = node_make_id(node2, "Float32");
                if (!node3)
                {
                    return -1;
                }
                node3->flag = NODE_FLAG_TEMPORARY;

                list_t *response1 = list_create();
                if (response1 == NULL)
                {
                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                    return -1;
                }

                int32_t r1 = semantic_select(program, node3->parent, NULL, node3, response1, SELECT_FLAG_NONE);
                if (r1 == -1)
                {
                    return -1;
                }
                if (r1 == 0)
                {
                    semantic_error(program, node3->parent, "reference not found");
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
                            node_t *clone1 = node_clone(item1->parent, item1);
                            if (clone1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }
                            clone1->flag |= NODE_FLAG_NEW;

                            ilist_t *il1 = list_rpush(response, clone1);
                            if (il1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }
                            node_prug(node3);
                            list_destroy(response1);
                            return 1;
                        }
                        else
                        if (item1->kind == NODE_KIND_PACKAGE)
                        {
                            node_package_t *package1 = (node_package_t *)item1->value;

                            node_t *root1 = item1->parent;
                            while (root1 != NULL)
                            {
                                if (root1->kind == NODE_KIND_IMPORT)
                                {
                                    break;
                                }
                                else
                                {
                                    root1 = root1->parent;
                                }
                            }

                            if (root1->kind != NODE_KIND_IMPORT)
                            {
                                semantic_error(program, root1, "import not found\n");
                                return -1;
                            }

                            node_import_t *import1 = (node_import_t *)root1->value;

                            node_basic_t *basic2 = (node_basic_t *)import1->path->value;

                            node_t *module1 = program_load(program, basic2->value);
                            if (module1 == NULL)
                            {
                                return -1;
                            }

                            list_t *response2 = list_create();
                            if (response2 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r2 = semantic_postfix(program, module1, package1->route, response2, SELECT_FLAG_NONE);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 0)
                            {
                                semantic_error(program, package1->route, "reference not found");
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
                                        node_t *clone2 = node_clone(item2->parent, item2);
                                        if (clone2 == NULL)
                                        {
                                            return -1;
                                        }
                                        clone2->flag |= NODE_FLAG_NEW;

                                        ilist_t *il1 = list_rpush(response, clone2);
                                        if (il1 == NULL)
                                        {
                                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }
                                    else
                                    {
                                        semantic_error(program, item2, "non-class");
                                        return -1;
                                    }
                                    return 1;

                                }
                            }
                            list_destroy(response2);
                        }
                        else
                        {
                            semantic_error(program, item1, "reference not found");
                            return -1;
                        }
                    }
                }
                node_prug(node3);
                list_destroy(response1);
                return 0;
            }
            else
            {
                // double
                node_t *node2 = node_create(node, node->position);
                if (node2 == NULL)
                {
                    return -1;
                }
                node_t *node3 = node_make_id(node2, "Float64");
                if (!node3)
                {
                    return -1;
                }
                node3->flag = NODE_FLAG_TEMPORARY;

                list_t *response1 = list_create();
                if (response1 == NULL)
                {
                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                    return -1;
                }

                int32_t r1 = semantic_select(program, node3->parent, NULL, node3, response1, SELECT_FLAG_NONE);
                if (r1 == -1)
                {
                    return -1;
                }
                if (r1 == 0)
                {
                    semantic_error(program, node3->parent, "reference not found");
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
                            node_t *clone1 = node_clone(item1->parent, item1);
                            if (clone1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }
                            clone1->flag |= NODE_FLAG_NEW;

                            ilist_t *il1 = list_rpush(response, clone1);
                            if (il1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }
                            node_prug(node3);
                            list_destroy(response1);
                            return 1;
                        }
                        else
                        if (item1->kind == NODE_KIND_PACKAGE)
                        {
                            node_package_t *package1 = (node_package_t *)item1->value;

                            node_t *root1 = item1->parent;
                            while (root1 != NULL)
                            {
                                if (root1->kind == NODE_KIND_IMPORT)
                                {
                                    break;
                                }
                                else
                                {
                                    root1 = root1->parent;
                                }
                            }

                            if (root1->kind != NODE_KIND_IMPORT)
                            {
                                semantic_error(program, root1, "import not found\n");
                                return -1;
                            }

                            node_import_t *import1 = (node_import_t *)root1->value;

                            node_basic_t *basic2 = (node_basic_t *)import1->path->value;

                            node_t *module1 = program_load(program, basic2->value);
                            if (module1 == NULL)
                            {
                                return -1;
                            }

                            list_t *response2 = list_create();
                            if (response2 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r2 = semantic_postfix(program, module1, package1->route, response2, SELECT_FLAG_NONE);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 0)
                            {
                                semantic_error(program, package1->route, "reference not found");
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
                                        node_t *clone2 = node_clone(item2->parent, item2);
                                        if (clone2 == NULL)
                                        {
                                            return -1;
                                        }
                                        clone2->flag |= NODE_FLAG_NEW;

                                        ilist_t *il1 = list_rpush(response, clone2);
                                        if (il1 == NULL)
                                        {
                                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }
                                    else
                                    {
                                        semantic_error(program, item2, "non-class");
                                        return -1;
                                    }
                                    return 1;

                                }
                            }
                            list_destroy(response2);
                        }
                        else
                        {
                            semantic_error(program, item1, "reference not found");
                            return -1;
                        }
                    }
                }
                node_prug(node3);
                list_destroy(response1);
                return 0;
            }
        }
    }
    return 0;
}

static int32_t
semantic_char(program_t *program, node_t *scope, node_t *node, list_t *response)
{
    node_t *node2 = node_create(node, node->position);
    if (node2 == NULL)
    {
        return -1;
    }
    node_t *node3 = node_make_id(node2, "Char");
    if (!node3)
    {
        return -1;
    }
    node3->flag = NODE_FLAG_TEMPORARY;

    list_t *response1 = list_create();
    if (response1 == NULL)
    {
        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    int32_t r1 = semantic_select(program, node3->parent, NULL, node3, response1, SELECT_FLAG_NONE);
    if (r1 == -1)
    {
        return -1;
    }
    if (r1 == 0)
    {
        semantic_error(program, node3->parent, "reference not found");
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
                node_t *clone1 = node_clone(item1->parent, item1);
                if (clone1 == NULL)
                {
                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                    return -1;
                }
                clone1->flag |= NODE_FLAG_NEW;

                ilist_t *il1 = list_rpush(response, clone1);
                if (il1 == NULL)
                {
                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                    return -1;
                }
                node_prug(node3);
                list_destroy(response1);
                return 1;
            }
            else
            if (item1->kind == NODE_KIND_PACKAGE)
            {
                node_package_t *package1 = (node_package_t *)item1->value;

                node_t *root1 = item1->parent;
                while (root1 != NULL)
                {
                    if (root1->kind == NODE_KIND_IMPORT)
                    {
                        break;
                    }
                    else
                    {
                        root1 = root1->parent;
                    }
                }

                if (root1->kind != NODE_KIND_IMPORT)
                {
                    semantic_error(program, root1, "import not found\n");
                    return -1;
                }

                node_import_t *import1 = (node_import_t *)root1->value;

                node_basic_t *basic2 = (node_basic_t *)import1->path->value;

                node_t *module1 = program_load(program, basic2->value);
                if (module1 == NULL)
                {
                    return -1;
                }

                list_t *response2 = list_create();
                if (response2 == NULL)
                {
                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                    return -1;
                }

                int32_t r2 = semantic_postfix(program, module1, package1->route, response2, SELECT_FLAG_NONE);
                if (r2 == -1)
                {
                    return -1;
                }
                else
                if (r2 == 0)
                {
                    semantic_error(program, package1->route, "reference not found");
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
                            node_t *clone2 = node_clone(item2->parent, item2);
                            if (clone2 == NULL)
                            {
                                return -1;
                            }
                            clone2->flag |= NODE_FLAG_NEW;

                            ilist_t *il1 = list_rpush(response, clone2);
                            if (il1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }
                        }
                        else
                        {
                            semantic_error(program, item2, "non-class");
                            return -1;
                        }
                        return 1;

                    }
                }
                list_destroy(response2);
            }
            else
            {
                semantic_error(program, item1, "reference not found");
                return -1;
            }
        }
    }
    node_prug(node3);
    list_destroy(response1);
    return 0;
}

static int32_t
semantic_string(program_t *program, node_t *scope, node_t *node, list_t *response)
{
    node_t *node2 = node_create(node, node->position);
    if (node2 == NULL)
    {
        return -1;
    }
    node_t *node3 = node_make_id(node2, "Array");
    if (!node3)
    {
        return -1;
    }
    node3->flag = NODE_FLAG_TEMPORARY;

    list_t *response1 = list_create();
    if (response1 == NULL)
    {
        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    int32_t r1 = semantic_select(program, node3->parent, NULL, node3, response1, SELECT_FLAG_NONE);
    if (r1 == -1)
    {
        return -1;
    }
    if (r1 == 0)
    {
        semantic_error(program, node3->parent, "reference not found");
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
                node_t *node4 = node_create(node, node->position);
                if (node4 == NULL)
                {
                    return -1;
                }
                node_t *node5 = node_make_id(node4, "Char");
                if (!node5)
                {
                    return -1;
                }
                node5->flag = NODE_FLAG_TEMPORARY;

                node_t *data = semantic_make_fields(program, node, 1, node5);
                if (data == NULL)
                {
                    return -1;
                }

                node_class_t *class1 = (node_class_t *)item1->value;
                node_t *ngs1 = class1->generics;
                node_t *nfs2 = data;
                int32_t r1 = semantic_eqaul_gsfs(program, ngs1, nfs2);
                if (r1 == -1)
                {
                    return -1;
                }
                else
                if (r1 == 1)
                {
					node_t *clone1 = node_clone(item1->parent, item1);
					if (clone1 == NULL)
					{
						return -1;
					}
                    clone1->flag |= NODE_FLAG_NEW;

					node_class_t *class2 = (node_class_t *)clone1->value;
					node_t *ngs3 = class2->generics;
					node_t *nfs4 = data;
					int32_t r2 = semantic_substitute_gsfs(program, ngs3, nfs4);
					if (r2 == -1)
					{
						return -1;
					}
					else
					if (r2 == 1)
					{
						ilist_t *il1 = list_rpush(response, clone1);
						if (il1 == NULL)
						{
							fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
							return -1;
						}
                        node_prug(node3);
                        list_destroy(response1);
                        return 1;
					}
                }
                node_prug(data);
                node_prug(node4);
            }
            else
            if (item1->kind == NODE_KIND_PACKAGE)
            {
                node_t *node4 = node_create(node, node->position);
                if (node4 == NULL)
                {
                    return -1;
                }
                node_t *node5 = node_make_id(node4, "Char");
                if (!node5)
                {
                    return -1;
                }
                node5->flag = NODE_FLAG_TEMPORARY;

                node_t *data = semantic_make_fields(program, node, 1, node5);
                if (data == NULL)
                {
                    return -1;
                }
                
                node_package_t *package2 = (node_package_t *)item1->value;
                node_t *ngs1 = package2->generics;
                node_t *nfs2 = data;
                int32_t r2 = semantic_eqaul_gsfs(program, ngs1, nfs2);
                if (r2 == -1)
                {
                    return -1;
                }
                else
                if (r2 == 1)
                {
					node_t *clone1 = node_clone(item1->parent, item1);
					if (clone1 == NULL)
					{
						return -1;
					}
					node_package_t *package3 = (node_package_t *)clone1->value;
					node_t *ngs3 = package3->generics;
					node_t *nfs4 = data;
					int32_t r3 = semantic_substitute_gsfs(program, ngs3, nfs4);
					if (r3 == -1)
					{
						return -1;
					}
					else
					if (r3 == 1)
					{
                        node_t *root1 = clone1->parent;
                        while (root1 != NULL)
                        {
                            if (root1->kind == NODE_KIND_IMPORT)
                            {
                                break;
                            }
                            else
                            {
                                root1 = root1->parent;
                            }
                        }

                        if (root1->kind != NODE_KIND_IMPORT)
                        {
                            semantic_error(program, root1, "import not found\n");
                            return -1;
                        }

                        node_import_t *import1 = (node_import_t *)root1->value;

                        node_basic_t *basic2 = (node_basic_t *)import1->path->value;

                        node_t *module1 = program_load(program, basic2->value);
                        if (module1 == NULL)
                        {
                            return -1;
                        }

                        list_t *response2 = list_create();
                        if (response2 == NULL)
                        {
                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }

                        int32_t r4 = semantic_postfix(program, module1, package3->route, response2, SELECT_FLAG_NONE);
                        if (r4 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r4 == 0)
                        {
                            semantic_error(program, package3->route, "reference not found");
                            return -1;
                        }
                        else
                        if (r4 == 1)
                        {
                            ilist_t *b1;
                            for (b1 = response2->begin;b1 != response2->end;b1 = b1->next)
                            {
                                node_t *item2 = (node_t *)b1->value;
                                if (item2->kind == NODE_KIND_CLASS)
                                {
                                    node_t *clone2 = node_clone(item2->parent, item2);
                                    if (clone2 == NULL)
                                    {
                                        return -1;
                                    }
                                    clone2->flag |= NODE_FLAG_NEW;

                                    ilist_t *il1 = list_rpush(response, clone2);
                                    if (il1 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                }
                                else
                                {
                                    semantic_error(program, item2, "non-class as generic");
                                    return -1;
                                }
                                return 1;

                            }
                        }
                        list_destroy(response2);

					}
                }
                node_prug(data);
                node_prug(node4);
            }
        }
    }
    node_prug(node3);
    list_destroy(response1);
    return 0;
}

static int32_t
semantic_null(program_t *program, node_t *scope, node_t *node, list_t *response)
{
    node_t *node2 = node_create(node, node->position);
    if (node2 == NULL)
    {
        return -1;
    }
    node_t *node3 = node_make_id(node2, "Null");
    if (!node3)
    {
        return -1;
    }
    node3->flag = NODE_FLAG_TEMPORARY;

    list_t *response1 = list_create();
    if (response1 == NULL)
    {
        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    int32_t r1 = semantic_select(program, node3->parent, NULL, node3, response1, SELECT_FLAG_NONE);
    if (r1 == -1)
    {
        return -1;
    }
    if (r1 == 0)
    {
        semantic_error(program, node3->parent, "reference not found");
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
                node_t *clone1 = node_clone(item1->parent, item1);
                if (clone1 == NULL)
                {
                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                    return -1;
                }
                clone1->flag |= NODE_FLAG_NEW;

                ilist_t *il1 = list_rpush(response, clone1);
                if (il1 == NULL)
                {
                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                    return -1;
                }
                node_prug(node3);
                list_destroy(response1);
                return 1;
            }
            else
            if (item1->kind == NODE_KIND_PACKAGE)
            {
                node_package_t *package1 = (node_package_t *)item1->value;

                node_t *root1 = item1->parent;
                while (root1 != NULL)
                {
                    if (root1->kind == NODE_KIND_IMPORT)
                    {
                        break;
                    }
                    else
                    {
                        root1 = root1->parent;
                    }
                }

                if (root1->kind != NODE_KIND_IMPORT)
                {
                    semantic_error(program, root1, "import not found\n");
                    return -1;
                }

                node_import_t *import1 = (node_import_t *)root1->value;

                node_basic_t *basic2 = (node_basic_t *)import1->path->value;

                node_t *module1 = program_load(program, basic2->value);
                if (module1 == NULL)
                {
                    return -1;
                }

                list_t *response2 = list_create();
                if (response2 == NULL)
                {
                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                    return -1;
                }

                int32_t r2 = semantic_postfix(program, module1, package1->route, response2, SELECT_FLAG_NONE);
                if (r2 == -1)
                {
                    return -1;
                }
                else
                if (r2 == 0)
                {
                    semantic_error(program, package1->route, "reference not found");
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
                            node_t *clone2 = node_clone(item2->parent, item2);
                            if (clone2 == NULL)
                            {
                                return -1;
                            }
                            clone2->flag |= NODE_FLAG_NEW;

                            ilist_t *il1 = list_rpush(response, clone2);
                            if (il1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }
                        }
                        else
                        {
                            semantic_error(program, item2, "non-class");
                            return -1;
                        }
                        return 1;

                    }
                }
                list_destroy(response2);
            }
            else
            {
                semantic_error(program, item1, "reference not found");
                return -1;
            }
        }
    }
    node_prug(node3);
    list_destroy(response1);
    return 0;
}

static int32_t
semantic_array(program_t *program, node_t *scope, node_t *node, list_t *response)
{
    node_array_t *array1 = (node_array_t *)node->value;

    ilist_t *a1;
    for (a1 = array1->list->begin;a1 != array1->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;

        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_postfix(program, NULL, item1, response1, SELECT_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        else
        if (r1 == 0)
        {
            semantic_error(program, item1, "reference not found");
            return -1;
        }
        else
        if (r1 == 1)
        {
            ilist_t *a2;
            for (a2 = response1->begin;a2 != response1->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                
            }
        }
    }
    return 1;
}

static int32_t
semantic_dictionary(program_t *program, node_t *scope, node_t *node, list_t *response)
{
    return 1;
}

/*
 * @Parameters:
 *      scope used for select as root
 *      node is current node
 * @Return: 1 if found, 0 if not found, -1 if wrong
*/
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

/*
 * @Parameters:
 *      scope used for select as root
 *      node is current node
 * @Return: 1 if found, 0 if not found, -1 if wrong
*/
static int32_t
semantic_pseudonym(program_t *program, node_t *scope, node_t *node, list_t *response, uint64_t flag)
{
    node_carrier_t *carrier = (node_carrier_t *)node->value;
    
    if (carrier->data != NULL)
    {
        int32_t r1 = semantic_fields(program, carrier->data);
        if (r1 == -1)
        {
            return -1;
        }
    }

    list_t *response1 = list_create();
    if (response1 == NULL)
    {
        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    int32_t r1 = semantic_postfix(program, scope, carrier->base, response1, flag);
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
					node_t *item2 = node_clone(item1->parent, item1);
					if (item2 == NULL)
					{
						return -1;
					}

					node_class_t *class2 = (node_class_t *)item2->value;
					node_t *ngs3 = class2->generics;
					node_t *nfs4 = carrier->data;
					int32_t r2 = semantic_substitute_gsfs(program, ngs3, nfs4);
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
							fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
							return -1;
						}
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
					node_t *item2 = node_clone(item1->parent, item1);
					if (item2 == NULL)
					{
						return -1;
					}

					node_package_t *package3 = (node_package_t *)item2->value;
					node_t *ngs3 = package3->generics;
					node_t *nfs4 = carrier->data;
					int32_t r2 = semantic_substitute_gsfs(program, ngs3, nfs4);
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
							fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
							return -1;
						}
					}
                }
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

/*
 * @Parameters:
 *      scope used for select as root
 *      node is current node
 * @Return: 1 if found, 0 if not found, -1 if wrong
*/
static int32_t
semantic_attribute(program_t *program, node_t *scope, node_t *node, list_t *response, uint64_t flag)
{
    node_binary_t *basic = (node_binary_t *)node->value;
    
    list_t *response1 = list_create();
    if (response1 == NULL)
    {
        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    int32_t r1 = semantic_resolve(program, scope, basic->left, response1, flag);
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
                            }
                            else
                            {
                                item2->flag &= ~NODE_FLAG_DERIVE;
                            }

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
                                    semantic_error(program, item2, "private access");
                                    return -1;
                                }

                                if ((enum2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                {
                                    semantic_error(program, item1, "protect access");
                                    return -1;
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
                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }
                            list_destroy(response1);
                            return 1;
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
                                    semantic_error(program, item2, "private access");
                                    return -1;
                                }

                                if ((fun2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                {
                                    semantic_error(program, item1, "protect access");
                                    return -1;
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
                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }
                            list_destroy(response1);
                            return 1;
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
                                    semantic_error(program, item2, "private access");
                                    return -1;
                                }

                                if ((property2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                {
                                    semantic_error(program, item1, "protect access");
                                    return -1;
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
                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }
                            list_destroy(response1);
                            return 1;
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
                                
                                    int32_t r4 = semantic_postfix(program, NULL, heritage4->type, response4, SELECT_FLAG_NONE);
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
                                                            }
                                                            else
                                                            {
                                                                item6->flag &= ~NODE_FLAG_DERIVE;
                                                            }

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
                                                            }
                                                            else
                                                            {
                                                                item6->flag &= ~NODE_FLAG_DERIVE;
                                                            }

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
                                                    else
                                                    if (item6->kind == NODE_KIND_FUN)
                                                    {
                                                        node_fun_t *fun1 = (node_fun_t *)item6->value;
                                                        if (semantic_idcmp(fun1->key, basic->right) == 1)
                                                        {
                                                            if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                            {
                                                                if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                {
                                                                    semantic_error(program, item6, "private access");
                                                                    return -1;
                                                                }

                                                                if ((fun1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                {
                                                                    semantic_error(program, item6, "protect access");
                                                                    return -1;
                                                                }

                                                                item6->flag |= NODE_FLAG_DERIVE;
                                                            }
                                                            else
                                                            {
                                                                item6->flag &= ~NODE_FLAG_DERIVE;
                                                            }

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
                                                    else
                                                    if (item6->kind == NODE_KIND_PROPERTY)
                                                    {
                                                        node_property_t *property1 = (node_property_t *)item6->value;
                                                        if (semantic_idcmp(property1->key, basic->right) == 1)
                                                        {
                                                            if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                            {
                                                                if ((property1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                {
                                                                    semantic_error(program, item6, "private access");
                                                                    return -1;
                                                                }

                                                                if ((property1->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                {
                                                                    semantic_error(program, item6, "protect access");
                                                                    return -1;
                                                                }

                                                                item6->flag |= NODE_FLAG_DERIVE;
                                                            }
                                                            else
                                                            {
                                                                item6->flag &= ~NODE_FLAG_DERIVE;
                                                            }

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
                                                if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                                                {
                                                    if ((class6->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                    {
                                                        semantic_error(program, item6, "non static");
                                                        return -1;
                                                    }
                                                }

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
                                                }
                                                else
                                                {
                                                    item6->flag &= ~NODE_FLAG_DERIVE;
                                                }

                                                ilist_t *r6 = list_rpush(response, item6);
                                                if (r6 == NULL)
                                                {
                                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                return 1;
                                            }
                                        }
                                        else
                                        if (item6->kind == NODE_KIND_ENUM)
                                        {
                                            node_enum_t *enum1 = (node_enum_t *)item6->value;
                                            if (semantic_idcmp(enum1->key, basic->right) == 1)
                                            {
                                                if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                                                {
                                                    if ((enum1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                    {
                                                        semantic_error(program, item6, "non static");
                                                        return -1;
                                                    }
                                                }
  

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
                                                }
                                                else
                                                {
                                                    item6->flag &= ~NODE_FLAG_DERIVE;
                                                }

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
                node_var_t *var1 = (node_var_t *)item1->value;
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
                        node_t *item3 = (node_t *)a2->value;
                        if (item3->kind == NODE_KIND_CLASS)
                        {
                            node_class_t *class2 = (node_class_t *)item3->value;
                            if (semantic_idcmp(basic->right, class2->key) == 1)
                            {
                                if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                                {
                                    if ((class2->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                    {
                                        semantic_error(program, item3, "non static");
                                        return -1;
                                    }
                                }

                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                {
                                    if ((class2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                    {
                                        semantic_error(program, item3, "private access");
                                        return -1;
                                    }

                                    if ((class2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                    {
                                        semantic_error(program, item3, "protect access");
                                        return -1;
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
                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                list_destroy(response1);
                                return 1;
                            }
                        }
                        else
                        if (item3->kind == NODE_KIND_ENUM)
                        {
                            node_enum_t *enum2 = (node_enum_t *)item3->value;
                            if (semantic_idcmp(basic->right, enum2->key) == 1)
                            {
                                if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                                {
                                    if ((enum2->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                    {
                                        semantic_error(program, item3, "non static");
                                        return -1;
                                    }
                                }

                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                {
                                    if ((enum2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                    {
                                        semantic_error(program, item3, "private access");
                                        return -1;
                                    }

                                    if ((enum2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                    {
                                        semantic_error(program, item3, "protect access");
                                        return -1;
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
                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                list_destroy(response1);
                                return 1;
                            }
                        }
                        else
                        if (item3->kind == NODE_KIND_FUN)
                        {
                            node_fun_t *fun2 = (node_fun_t *)item3->value;
                            if (semantic_idcmp(basic->right, fun2->key) == 1)
                            {
                                if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                                {
                                    if ((fun2->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                    {
                                        semantic_error(program, item3, "non static");
                                        return -1;
                                    }
                                }

                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                {
                                    if ((fun2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                    {
                                        semantic_error(program, item3, "private access");
                                        return -1;
                                    }

                                    if ((fun2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                    {
                                        semantic_error(program, item3, "protect access");
                                        return -1;
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
                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                list_destroy(response1);
                                return 1;
                            }
                        }
                        else
                        if (item3->kind == NODE_KIND_PROPERTY)
                        {
                            node_property_t *property2 = (node_property_t *)item3->value;
                            if (semantic_idcmp(basic->right, property2->key) == 1)
                            {
                                if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                                {
                                    if ((property2->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                    {
                                        semantic_error(program, item3, "non static");
                                        return -1;
                                    }
                                }

                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                {
                                    if ((property2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                    {
                                        semantic_error(program, item3, "private access");
                                        return -1;
                                    }

                                    if ((property2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                    {
                                        semantic_error(program, item3, "protect access");
                                        return -1;
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
                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                list_destroy(response1);
                                return 1;
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
                                        
                                        int32_t r4 = semantic_postfix(program, NULL, heritage4->type, response4, SELECT_FLAG_NONE);
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
                                                                }
                                                                else
                                                                {
                                                                    item6->flag &= ~NODE_FLAG_DERIVE;
                                                                }

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
                                                        else
                                                        if (item6->kind == NODE_KIND_ENUM)
                                                        {
                                                            node_enum_t *enum6 = (node_enum_t *)item6->value;
                                                            if (semantic_idcmp(enum6->key, basic->right) == 1)
                                                            {
                                                                if ((item2->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
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
                                                                }
                                                                else
                                                                {
                                                                    item6->flag &= ~NODE_FLAG_DERIVE;
                                                                }

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
                                                        else
                                                        if (item6->kind == NODE_KIND_FUN)
                                                        {
                                                            node_fun_t *fun6 = (node_fun_t *)item6->value;
                                                            if (semantic_idcmp(fun6->key, basic->right) == 1)
                                                            {
                                                                if ((item2->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                {
                                                                    if ((fun6->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                    {
                                                                        semantic_error(program, item6, "private access");
                                                                        return -1;
                                                                    }

                                                                    if ((fun6->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                    {
                                                                        semantic_error(program, item6, "protect access");
                                                                        return -1;
                                                                    }

                                                                    item6->flag |= NODE_FLAG_DERIVE;
                                                                }
                                                                else
                                                                {
                                                                    item6->flag &= ~NODE_FLAG_DERIVE;
                                                                }

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
                                                        else
                                                        if (item6->kind == NODE_KIND_PROPERTY)
                                                        {
                                                            node_property_t *property6 = (node_property_t *)item6->value;
                                                            if (semantic_idcmp(property6->key, basic->right) == 1)
                                                            {
                                                                if ((item2->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                {
                                                                    if ((property6->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                    {
                                                                        semantic_error(program, item6, "private access");
                                                                        return -1;
                                                                    }

                                                                    if ((property6->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                    {
                                                                        semantic_error(program, item6, "protect access");
                                                                        return -1;
                                                                    }

                                                                    item6->flag |= NODE_FLAG_DERIVE;
                                                                }
                                                                else
                                                                {
                                                                    item6->flag &= ~NODE_FLAG_DERIVE;
                                                                }

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
                                                    }
                                                    else
                                                    {
                                                        item6->flag &= ~NODE_FLAG_DERIVE;
                                                    }

                                                    ilist_t *r6 = list_rpush(response, item6);
                                                    if (r6 == NULL)
                                                    {
                                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                    return 1;
                                                }
                                            }
                                            else
                                            if (item6->kind == NODE_KIND_ENUM)
                                            {
                                                node_enum_t *enum6 = (node_enum_t *)item6->value;
                                                if (semantic_idcmp(enum6->key, basic->right) == 1)
                                                {
                                                    if ((item2->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
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
                                                    }
                                                    else
                                                    {
                                                        item6->flag &= ~NODE_FLAG_DERIVE;
                                                    }

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
            if (item1->kind == NODE_KIND_ENTITY)
            {
                node_entity_t *entity1 = (node_entity_t *)item1->value;
                if (entity1->value_update == NULL)
                {
                    semantic_error(program, item1, "not initialized");
                    return -1;
                }

                node_t *item2 = entity1->value_update;
                if (item2->kind == NODE_KIND_CLASS)
                {
                    node_class_t *class1 = (node_class_t *)item2->value;
                    node_t *node1 = class1->block;
                    node_block_t *block1 = (node_block_t *)node1->value;
                    ilist_t *a2;
                    for (a2 = block1->list->begin;a2 != block1->list->end;a2 = a2->next)
                    {
                        node_t *item3 = (node_t *)a2->value;
                        if (item3->kind == NODE_KIND_CLASS)
                        {
                            node_class_t *class2 = (node_class_t *)item3->value;
                            if (semantic_idcmp(basic->right, class2->key) == 1)
                            {
                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                {
                                    if ((class2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                    {
                                        semantic_error(program, item3, "private access");
                                        return -1;
                                    }

                                    if ((class2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                    {
                                        semantic_error(program, item3, "protect access");
                                        return -1;
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
                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                list_destroy(response1);
                                return 1;
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
                                        semantic_error(program, item3, "private access");
                                        return -1;
                                    }

                                    if ((enum2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                    {
                                        semantic_error(program, item3, "protect access");
                                        return -1;
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
                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                list_destroy(response1);
                                return 1;
                            }
                        }
                        else
                        if (item3->kind == NODE_KIND_FUN)
                        {
                            node_fun_t *fun2 = (node_fun_t *)item3->value;
                            if (semantic_idcmp(basic->right, fun2->key) == 1)
                            {
                                if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                                {
                                    if ((fun2->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                    {
                                        semantic_error(program, item3, "non static");
                                        return -1;
                                    }
                                }

                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                {
                                    if ((fun2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                    {
                                        semantic_error(program, item3, "private access");
                                        return -1;
                                    }

                                    if ((fun2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                    {
                                        semantic_error(program, item3, "protect access");
                                        return -1;
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
                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                list_destroy(response1);
                                return 1;
                            }
                        }
                        else
                        if (item3->kind == NODE_KIND_PROPERTY)
                        {
                            node_property_t *property2 = (node_property_t *)item3->value;
                            if (semantic_idcmp(basic->right, property2->key) == 1)
                            {
                                if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                                {
                                    if ((property2->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                    {
                                        semantic_error(program, item3, "non static");
                                        return -1;
                                    }
                                }

                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                {
                                    if ((property2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                    {
                                        semantic_error(program, item3, "private access");
                                        return -1;
                                    }

                                    if ((property2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                    {
                                        semantic_error(program, item3, "protect access");
                                        return -1;
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
                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                list_destroy(response1);
                                return 1;
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
                                        
                                        int32_t r4 = semantic_postfix(program, NULL, heritage4->type, response4, SELECT_FLAG_NONE);
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
                                                                }
                                                                else
                                                                {
                                                                    item6->flag &= ~NODE_FLAG_DERIVE;
                                                                }

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
                                                        else
                                                        if (item6->kind == NODE_KIND_ENUM)
                                                        {
                                                            node_enum_t *enum6 = (node_enum_t *)item6->value;
                                                            if (semantic_idcmp(enum6->key, basic->right) == 1)
                                                            {
                                                                if ((item2->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
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
                                                                }
                                                                else
                                                                {
                                                                    item6->flag &= ~NODE_FLAG_DERIVE;
                                                                }

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
                                                        else
                                                        if (item6->kind == NODE_KIND_FUN)
                                                        {
                                                            node_fun_t *fun6 = (node_fun_t *)item6->value;
                                                            if (semantic_idcmp(fun6->key, basic->right) == 1)
                                                            {
                                                                if ((item2->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                {
                                                                    if ((fun6->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                    {
                                                                        semantic_error(program, item6, "private access");
                                                                        return -1;
                                                                    }

                                                                    if ((fun6->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                    {
                                                                        semantic_error(program, item6, "protect access");
                                                                        return -1;
                                                                    }

                                                                    item6->flag |= NODE_FLAG_DERIVE;
                                                                }
                                                                else
                                                                {
                                                                    item6->flag &= ~NODE_FLAG_DERIVE;
                                                                }

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
                                                        else
                                                        if (item6->kind == NODE_KIND_PROPERTY)
                                                        {
                                                            node_property_t *property6 = (node_property_t *)item6->value;
                                                            if (semantic_idcmp(property6->key, basic->right) == 1)
                                                            {
                                                                if ((item2->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                {
                                                                    if ((property6->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                    {
                                                                        semantic_error(program, item6, "private access");
                                                                        return -1;
                                                                    }

                                                                    if ((property6->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                    {
                                                                        semantic_error(program, item6, "protect access");
                                                                        return -1;
                                                                    }

                                                                    item6->flag |= NODE_FLAG_DERIVE;
                                                                }
                                                                else
                                                                {
                                                                    item6->flag &= ~NODE_FLAG_DERIVE;
                                                                }

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
                                                    }
                                                    else
                                                    {
                                                        item6->flag &= ~NODE_FLAG_DERIVE;
                                                    }

                                                    ilist_t *r6 = list_rpush(response, item6);
                                                    if (r6 == NULL)
                                                    {
                                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                    return 1;
                                                }
                                            }
                                            else
                                            if (item6->kind == NODE_KIND_ENUM)
                                            {
                                                node_enum_t *enum6 = (node_enum_t *)item6->value;
                                                if (semantic_idcmp(enum6->key, basic->right) == 1)
                                                {
                                                    if ((item2->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
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
                                                    }
                                                    else
                                                    {
                                                        item6->flag &= ~NODE_FLAG_DERIVE;
                                                    }

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
                node_property_t *property1 = (node_property_t *)item1->value;
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
                        node_t *item3 = (node_t *)a2->value;
                        if (item3->kind == NODE_KIND_CLASS)
                        {
                            node_class_t *class2 = (node_class_t *)item3->value;
                            if (semantic_idcmp(basic->right, class2->key) == 1)
                            {
                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                {
                                    if ((class2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                    {
                                        semantic_error(program, item3, "private access");
                                        return -1;
                                    }

                                    if ((class2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                    {
                                        semantic_error(program, item3, "protect access");
                                        return -1;
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
                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                list_destroy(response1);
                                return 1;
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
                                        semantic_error(program, item3, "private access");
                                        return -1;
                                    }

                                    if ((enum2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                    {
                                        semantic_error(program, item3, "protect access");
                                        return -1;
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
                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                list_destroy(response1);
                                return 1;
                            }
                        }
                        else
                        if (item3->kind == NODE_KIND_FUN)
                        {
                            node_fun_t *fun2 = (node_fun_t *)item3->value;
                            if (semantic_idcmp(basic->right, fun2->key) == 1)
                            {
                                if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                                {
                                    if ((fun2->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                    {
                                        semantic_error(program, item3, "non static");
                                        return -1;
                                    }
                                }

                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                {
                                    if ((fun2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                    {
                                        semantic_error(program, item3, "private access");
                                        return -1;
                                    }

                                    if ((fun2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                    {
                                        semantic_error(program, item3, "protect access");
                                        return -1;
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
                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                list_destroy(response1);
                                return 1;
                            }
                        }
                        else
                        if (item3->kind == NODE_KIND_PROPERTY)
                        {
                            node_property_t *property2 = (node_property_t *)item3->value;
                            if (semantic_idcmp(basic->right, property2->key) == 1)
                            {
                                if ((item1->flag & NODE_FLAG_NEW) != NODE_FLAG_NEW)
                                {
                                    if ((property2->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                    {
                                        semantic_error(program, item3, "non static");
                                        return -1;
                                    }
                                }

                                if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                {
                                    if ((property2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                    {
                                        semantic_error(program, item3, "private access");
                                        return -1;
                                    }

                                    if ((property2->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                    {
                                        semantic_error(program, item3, "protect access");
                                        return -1;
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
                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                list_destroy(response1);
                                return 1;
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
                                        
                                        int32_t r4 = semantic_postfix(program, NULL, heritage4->type, response4, SELECT_FLAG_NONE);
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
                                                                }
                                                                else
                                                                {
                                                                    item6->flag &= ~NODE_FLAG_DERIVE;
                                                                }

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
                                                        else
                                                        if (item6->kind == NODE_KIND_ENUM)
                                                        {
                                                            node_enum_t *enum6 = (node_enum_t *)item6->value;
                                                            if (semantic_idcmp(enum6->key, basic->right) == 1)
                                                            {
                                                                if ((item2->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
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
                                                                }
                                                                else
                                                                {
                                                                    item6->flag &= ~NODE_FLAG_DERIVE;
                                                                }

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
                                                        else
                                                        if (item6->kind == NODE_KIND_FUN)
                                                        {
                                                            node_fun_t *fun6 = (node_fun_t *)item6->value;
                                                            if (semantic_idcmp(fun6->key, basic->right) == 1)
                                                            {
                                                                if ((item2->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                {
                                                                    if ((fun6->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                    {
                                                                        semantic_error(program, item6, "private access");
                                                                        return -1;
                                                                    }

                                                                    if ((fun6->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                    {
                                                                        semantic_error(program, item6, "protect access");
                                                                        return -1;
                                                                    }

                                                                    item6->flag |= NODE_FLAG_DERIVE;
                                                                }
                                                                else
                                                                {
                                                                    item6->flag &= ~NODE_FLAG_DERIVE;
                                                                }

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
                                                        else
                                                        if (item6->kind == NODE_KIND_PROPERTY)
                                                        {
                                                            node_property_t *property6 = (node_property_t *)item6->value;
                                                            if (semantic_idcmp(property6->key, basic->right) == 1)
                                                            {
                                                                if ((item2->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                {
                                                                    if ((property6->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                    {
                                                                        semantic_error(program, item6, "private access");
                                                                        return -1;
                                                                    }

                                                                    if ((property6->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
                                                                    {
                                                                        semantic_error(program, item6, "protect access");
                                                                        return -1;
                                                                    }

                                                                    item6->flag |= NODE_FLAG_DERIVE;
                                                                }
                                                                else
                                                                {
                                                                    item6->flag &= ~NODE_FLAG_DERIVE;
                                                                }

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
                                                    }
                                                    else
                                                    {
                                                        item6->flag &= ~NODE_FLAG_DERIVE;
                                                    }

                                                    ilist_t *r6 = list_rpush(response, item6);
                                                    if (r6 == NULL)
                                                    {
                                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                    return 1;
                                                }
                                            }
                                            else
                                            if (item6->kind == NODE_KIND_ENUM)
                                            {
                                                node_enum_t *enum6 = (node_enum_t *)item6->value;
                                                if (semantic_idcmp(enum6->key, basic->right) == 1)
                                                {
                                                    if ((item2->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
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
                                                    }
                                                    else
                                                    {
                                                        item6->flag &= ~NODE_FLAG_DERIVE;
                                                    }

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
                
                    int32_t r2 = semantic_postfix(program, NULL, heritage1->type, response2, SELECT_FLAG_NONE);
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
                                    else
                                    if (item3->kind == NODE_KIND_FUN)
                                    {
                                        node_fun_t *fun3 = (node_fun_t *)item3->value;
                                        if (semantic_idcmp(fun3->key, basic->right) == 1)
                                        {
                                            if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                            {
                                                if ((fun3->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                {
                                                    semantic_error(program, item3, "private access");
                                                    return -1;
                                                }

                                                if ((fun3->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
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
                                    if (item3->kind == NODE_KIND_PROPERTY)
                                    {
                                        node_property_t *property3 = (node_property_t *)item3->value;
                                        if (semantic_idcmp(property3->key, basic->right) == 1)
                                        {
                                            if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                            {
                                                if ((property3->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                {
                                                    semantic_error(program, item3, "private access");
                                                    return -1;
                                                }

                                                if ((property3->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
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
                                            
                                                int32_t r4 = semantic_postfix(program, NULL, heritage4->type, response4, SELECT_FLAG_NONE);
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
                                                                else
                                                                if (item6->kind == NODE_KIND_FUN)
                                                                {
                                                                    node_fun_t *fun6 = (node_fun_t *)item6->value;
                                                                    if (semantic_idcmp(fun6->key, basic->right) == 1)
                                                                    {
                                                                        if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                        {
                                                                            if ((fun6->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                            {
                                                                                semantic_error(program, item6, "private access");
                                                                                return -1;
                                                                            }

                                                                            if ((fun6->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
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
                                                                if (item6->kind == NODE_KIND_PROPERTY)
                                                                {
                                                                    node_property_t *property6 = (node_property_t *)item6->value;
                                                                    if (semantic_idcmp(property6->key, basic->right) == 1)
                                                                    {
                                                                        if ((item1->flag & NODE_FLAG_DERIVE) == NODE_FLAG_DERIVE)
                                                                        {
                                                                            if ((property6->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                            {
                                                                                semantic_error(program, item6, "private access");
                                                                                return -1;
                                                                            }

                                                                            if ((property6->flag & SYNTAX_MODIFIER_PROTECT) == SYNTAX_MODIFIER_PROTECT)
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
                                semantic_error(program, item2, "non-class as heritage7, for (%lld:%lld)",
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

                    if (item2->kind == NODE_KIND_CLASS)
                    {
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
                                
                                    int32_t r4 = semantic_postfix(program, NULL, heritage4->type, response4, SELECT_FLAG_NONE);
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
                                                semantic_error(program, item5, "non-class as heritage8, for (%lld:%lld)",
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
            else
			{
				semantic_error(program, item1, "wrong access, in (%lld:%lld)",
					node->position.line, node->position.column);
				return -1;
			}
        }
    }

    list_destroy(response1);

    return 0;
}

static int32_t
semantic_call(program_t *program, node_t *scope, node_t *node, list_t *response, uint64_t flag)
{
    node_carrier_t *carrier = (node_carrier_t *)node->value;
    
    if (carrier->base->kind == NODE_KIND_PARENTHESIS)
    {
        node_unary_t *unary1 = (node_unary_t *)carrier->base;
        node_t *item1 = unary1->right;
        if (item1->kind == NODE_KIND_CLASS)
        {
            node_t *clone1 = node_clone(item1->parent, item1);
            if (clone1 == NULL)
            {
                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                return -1;
            }
            clone1->flag |= NODE_FLAG_NEW;

            ilist_t *il1 = list_rpush(response, clone1);
            if (il1 == NULL)
            {
                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                return -1;
            }
            return 1;
        }
        else
        {
            semantic_error(program, item1, "not castable");
            return -1;
        }
    }

    list_t *response1 = list_create();
    if (response1 == NULL)
    {
        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    int32_t r1 = semantic_resolve(program, scope, carrier->base, response1, flag | SELECT_FLAG_OBJECT);
    if (r1 == -1)
    {
        return -1;
    }
    else
    if (r1 == 0)
    {
        semantic_error(program, node, "reference not found");
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
                if ((item1->flag & NODE_FLAG_NEW) == NODE_FLAG_NEW)
                {
                    semantic_error(program, item1, "calling an object");
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
                            if (r2 == 1)
                            {
                                item1->flag |= NODE_FLAG_NEW;

                                ilist_t *il1 = list_rpush(response, item1);
                                if (il1 == NULL)
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
                if (r1 == 1)
                {
                    list_t *response2 = list_create();
                    if (response2 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r3 = semantic_resolve(program, NULL, fun1->result, response2, SELECT_FLAG_NONE);
                    if (r3 == -1)
                    {
                        return -1;
                    }
                    if (r3 == 0)
                    {
                        semantic_error(program, fun1->result, "reference not found");
                        return -1;
                    }
                    else
                    if (r3 == 1)
                    {
                        ilist_t *a3;
                        for (a3 = response2->begin;a3 != response2->end;a3 = a3->next)
                        {
                            node_t *item3 = (node_t *)a3->value;

                            if (item3->kind == NODE_KIND_CLASS)
                            {
                                item3->flag |= NODE_FLAG_NEW;
                            }

                            ilist_t *il1 = list_rpush(response, item3);
                            if (il1 == NULL)
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
                if (r1 == 1)
                {
                    list_t *response2 = list_create();
                    if (response2 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r3 = semantic_resolve(program, NULL, fun1->result, response2, SELECT_FLAG_NONE);
                    if (r3 == -1)
                    {
                        return -1;
                    }
                    if (r3 == 0)
                    {
                        semantic_error(program, fun1->result, "reference not found");
                        return -1;
                    }
                    else
                    if (r3 == 1)
                    {
                        ilist_t *a3;
                        for (a3 = response2->begin;a3 != response2->end;a3 = a3->next)
                        {
                            node_t *item3 = (node_t *)a3->value;

                            if (item3->kind == NODE_KIND_CLASS)
                            {
                                item3->flag |= NODE_FLAG_NEW;
                            }

                            ilist_t *il1 = list_rpush(response, item3);
                            if (il1 == NULL)
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
            if (item1->kind == NODE_KIND_PROPERTY)
            {
                node_property_t *property1 = (node_property_t *)item1->value;
                if (property1->value_update == NULL)
                {
                    semantic_error(program, item1, "not initialized");
                    return -1;
                }

                node_t *referto = property1->value_update;
                if (referto->kind == NODE_KIND_CLASS)
                {
                    if ((referto->flag & NODE_FLAG_NEW) == NODE_FLAG_NEW)
                    {
                        semantic_error(program, referto, "calling an object");
                        return -1;
                    }

                    node_class_t *class1 = (node_class_t *)referto->value;
                    node_t *node1 = class1->block;
                    node_block_t *block1 = (node_block_t *)node1->value;
                    ilist_t *b1;
                    for (b1 = block1->list->begin;b1 != block1->list->end;b1 = b1->next)
                    {
                        node_t *item2 = (node_t *)b1->value;
                        if (item2->kind == NODE_KIND_FUN)
                        {
                            node_fun_t *fun1 = (node_fun_t *)item2->value;
                            if (semantic_idstrcmp(fun1->key, "constructor") == 1)
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
                                    node_t *clone1 = node_clone(referto->parent, referto);
                                    if (clone1 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    clone1->flag |= NODE_FLAG_NEW;

                                    ilist_t *il1 = list_rpush(response, clone1);
                                    if (il1 == NULL)
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
                }
                else
                if (referto->kind == NODE_KIND_FUN)
                {
                    node_fun_t *fun1 = (node_fun_t *)referto->value;
                    node_t *nps1 = fun1->parameters;
                    node_t *nds2 = carrier->data;
                    int32_t r1 = semantic_eqaul_psas(program, nps1, nds2);
                    if (r1 == -1)
                    {
                        return -1;
                    }
                    else
                    if (r1 == 1)
                    {
                        list_t *response2 = list_create();
                        if (response2 == NULL)
                        {
                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }

                        int32_t r3 = semantic_resolve(program, NULL, fun1->result, response2, SELECT_FLAG_NONE);
                        if (r3 == -1)
                        {
                            return -1;
                        }
                        if (r3 == 0)
                        {
                            semantic_error(program, fun1->result, "reference not found");
                            return -1;
                        }
                        else
                        if (r3 == 1)
                        {
                            ilist_t *a3;
                            for (a3 = response2->begin;a3 != response2->end;a3 = a3->next)
                            {
                                node_t *item3 = (node_t *)a3->value;

                                node_t *clone1 = node_clone(item1->parent, item3);
                                if (clone1 == NULL)
                                {
                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }

                                if (item3->kind == NODE_KIND_CLASS)
                                {
                                    clone1->flag |= NODE_FLAG_NEW;
                                }

                                ilist_t *il1 = list_rpush(response, clone1);
                                if (il1 == NULL)
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

                node_t *referto = var1->value_update;
                if (referto->kind == NODE_KIND_CLASS)
                {
                    if ((referto->flag & NODE_FLAG_NEW) == NODE_FLAG_NEW)
                    {
                        semantic_error(program, referto, "calling an object");
                        return -1;
                    }

                    node_class_t *class1 = (node_class_t *)referto->value;
                    node_t *node1 = class1->block;
                    node_block_t *block1 = (node_block_t *)node1->value;
                    ilist_t *b1;
                    for (b1 = block1->list->begin;b1 != block1->list->end;b1 = b1->next)
                    {
                        node_t *item2 = (node_t *)b1->value;
                        if (item2->kind == NODE_KIND_FUN)
                        {
                            node_fun_t *fun1 = (node_fun_t *)item2->value;
                            if (semantic_idstrcmp(fun1->key, "constructor") == 1)
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
                                    node_t *clone1 = node_clone(referto->parent, referto);
                                    if (clone1 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    clone1->flag |= NODE_FLAG_NEW;

                                    ilist_t *il1 = list_rpush(response, clone1);
                                    if (il1 == NULL)
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
                }
                else
                if (referto->kind == NODE_KIND_FUN)
                {
                    node_fun_t *fun1 = (node_fun_t *)referto->value;
                    node_t *nps1 = fun1->parameters;
                    node_t *nds2 = carrier->data;
                    int32_t r1 = semantic_eqaul_psas(program, nps1, nds2);
                    if (r1 == -1)
                    {
                        return -1;
                    }
                    else
                    if (r1 == 1)
                    {
                        list_t *response2 = list_create();
                        if (response2 == NULL)
                        {
                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }

                        int32_t r3 = semantic_resolve(program, NULL, fun1->result, response2, SELECT_FLAG_NONE);
                        if (r3 == -1)
                        {
                            return -1;
                        }
                        if (r3 == 0)
                        {
                            semantic_error(program, fun1->result, "reference not found");
                            return -1;
                        }
                        else
                        if (r3 == 1)
                        {
                            ilist_t *a3;
                            for (a3 = response2->begin;a3 != response2->end;a3 = a3->next)
                            {
                                node_t *item3 = (node_t *)a3->value;

                                node_t *clone1 = node_clone(item1->parent, item3);
                                if (clone1 == NULL)
                                {
                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }

                                if (item3->kind == NODE_KIND_CLASS)
                                {
                                    clone1->flag |= NODE_FLAG_NEW;
                                }

                                ilist_t *il1 = list_rpush(response, clone1);
                                if (il1 == NULL)
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

                node_t *referto = entity1->value_update;
                if (referto->kind == NODE_KIND_CLASS)
                {
                    if ((referto->flag & NODE_FLAG_NEW) == NODE_FLAG_NEW)
                    {
                        semantic_error(program, referto, "calling an object");
                        return -1;
                    }

                    node_class_t *class1 = (node_class_t *)referto->value;
                    node_t *node1 = class1->block;
                    node_block_t *block1 = (node_block_t *)node1->value;
                    ilist_t *b1;
                    for (b1 = block1->list->begin;b1 != block1->list->end;b1 = b1->next)
                    {
                        node_t *item2 = (node_t *)b1->value;
                        if (item2->kind == NODE_KIND_FUN)
                        {
                            node_fun_t *fun1 = (node_fun_t *)item2->value;
                            if (semantic_idstrcmp(fun1->key, "constructor") == 1)
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
                                    node_t *clone1 = node_clone(referto->parent, referto);
                                    if (clone1 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    clone1->flag |= NODE_FLAG_NEW;

                                    ilist_t *il1 = list_rpush(response, clone1);
                                    if (il1 == NULL)
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
                }
                else
                if (referto->kind == NODE_KIND_FUN)
                {
                    node_fun_t *fun1 = (node_fun_t *)referto->value;
                    node_t *nps1 = fun1->parameters;
                    node_t *nds2 = carrier->data;
                    int32_t r1 = semantic_eqaul_psas(program, nps1, nds2);
                    if (r1 == -1)
                    {
                        return -1;
                    }
                    else
                    if (r1 == 1)
                    {
                        list_t *response2 = list_create();
                        if (response2 == NULL)
                        {
                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }

                        int32_t r3 = semantic_resolve(program, NULL, fun1->result, response2, SELECT_FLAG_NONE);
                        if (r3 == -1)
                        {
                            return -1;
                        }
                        if (r3 == 0)
                        {
                            semantic_error(program, fun1->result, "reference not found");
                            return -1;
                        }
                        else
                        if (r3 == 1)
                        {
                            ilist_t *a3;
                            for (a3 = response2->begin;a3 != response2->end;a3 = a3->next)
                            {
                                node_t *item3 = (node_t *)a3->value;

                                node_t *clone1 = node_clone(item1->parent, item3);
                                if (clone1 == NULL)
                                {
                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }

                                if (item3->kind == NODE_KIND_CLASS)
                                {
                                    clone1->flag |= NODE_FLAG_NEW;
                                }

                                ilist_t *il1 = list_rpush(response, clone1);
                                if (il1 == NULL)
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
                                if (semantic_idstrcmp(fun1->key, "constructor") == 1)
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
                                        node_t *clone1 = node_clone(item2->parent, item2);
                                        if (clone1 == NULL)
                                        {
                                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }
                                        clone1->flag |= NODE_FLAG_NEW;

                                        ilist_t *il1 = list_rpush(response, clone1);
                                        if (il1 == NULL)
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
                    }
                }
                else
                {
                    list_t *response2 = list_create();
                    if (response2 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                    int32_t r2 = semantic_postfix(program, NULL, heritage1->type, response2, SELECT_FLAG_NONE);
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
                                        if (semantic_idstrcmp(fun1->key, "constructor") == 1)
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
                                                node_t *clone1 = node_clone(item2->parent, item2);
                                                if (clone1 == NULL)
                                                {
                                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                clone1->flag |= NODE_FLAG_NEW;

                                                ilist_t *il1 = list_rpush(response, clone1);
                                                if (il1 == NULL)
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
        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    int32_t r1 = semantic_resolve(program, scope, carrier->base, response1, flag | SELECT_FLAG_OBJECT);
    if (r1 == -1)
    {
        return -1;
    }
    else
    if (r1 == 0)
    {
        semantic_error(program, node, "reference not found");
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
                                    node_t *clone1 = node_clone(item1->parent, item3);
                                    if (clone1 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    clone1->flag |= NODE_FLAG_NEW;

                                    ilist_t *il1 = list_rpush(response, clone1);
                                    if (il1 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
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
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
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

/*
 * @Parameters:
 *      scope used for select as root
 *      node is current node
 * @Return: 1 if found, 0 if not found, -1 if wrong
*/
static int32_t
semantic_postfix(program_t *program, node_t *scope, node_t *node, list_t *response, uint64_t flag)
{
    printf("wow1 %d\n", node->kind);
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
        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    int32_t r1 = semantic_postfix(program, NULL, node, response1, SELECT_FLAG_NONE);
    if (r1 == -1)
    {
        return -1;
    }
    if (r1 == 0)
    {
        semantic_error(program, node, "reference not found");
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
                    if (root1->kind == NODE_KIND_IMPORT)
                    {
                        break;
                    }
                    else
                    {
                        root1 = root1->parent;
                    }
                }

                if (root1->kind != NODE_KIND_IMPORT)
                {
                    semantic_error(program, root1, "import not found\n");
                    return -1;
                }

                node_import_t *import1 = (node_import_t *)root1->value;

                node_basic_t *basic2 = (node_basic_t *)import1->path->value;

                node_t *module1 = program_load(program, basic2->value);
                if (module1 == NULL)
                {
                    return -1;
                }

                list_t *response2 = list_create();
                if (response2 == NULL)
                {
                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                    return -1;
                }

                int32_t r2 = semantic_postfix(program, module1, package1->route, response2, SELECT_FLAG_NONE);
                if (r2 == -1)
                {
                    return -1;
                }
                else
                if (r2 == 0)
                {
                    semantic_error(program, package1->route, "reference not found");
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
                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
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
                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
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
semantic_prefix(program_t *program, node_t *scope, node_t *node, list_t *response, uint64_t flag)
{
    if (node->kind == NODE_KIND_TILDE)
    {
        node_unary_t *unary1 = (node_unary_t *)node->value;

        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_resolve(program, NULL, unary1->right, response1, SELECT_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        if (r1 == 0)
        {
            semantic_error(program, unary1->right, "reference not found");
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
                    node_class_t *class2 = (node_class_t *)item1->value;
                    node_block_t *block2 = class2->block->value;
                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_FUN)
                        {
                            node_fun_t *fun1 = (node_fun_t *)item2->value;
                            if (semantic_idstrcmp(fun1->key, "~") == 1)
                            {
                                node_t *nps1 = fun1->parameters;
                                node_t *nds2 = NULL;
                                int32_t r2 = semantic_eqaul_psas(program, nps1, nds2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r3 = semantic_resolve(program, NULL, fun1->result, response2, SELECT_FLAG_NONE);
                                    if (r3 == -1)
                                    {
                                        return -1;
                                    }
                                    if (r3 == 0)
                                    {
                                        semantic_error(program, fun1->result, "reference not found");
                                        return -1;
                                    }
                                    else
                                    if (r3 == 1)
                                    {
                                        ilist_t *a3;
                                        for (a3 = response2->begin;a3 != response2->end;a3 = a3->next)
                                        {
                                            node_t *item3 = (node_t *)a3->value;

                                            node_t *clone1 = node_clone(item3->parent, item3);
                                            if (clone1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            
                                            if (item3->kind == NODE_KIND_CLASS)
                                            {
                                                clone1->flag |= NODE_FLAG_NEW;
                                            }

                                            ilist_t *il1 = list_rpush(response, clone1);
                                            if (il1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }
                                    }
                                    list_destroy(response2);
                                }
                            }
                        }
                    }
                }
            }
        }
        list_destroy(response1);
        return 0;
    }
    else
    if (node->kind == NODE_KIND_POS)
    {
        node_unary_t *unary1 = (node_unary_t *)node->value;

        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_resolve(program, NULL, unary1->right, response1, SELECT_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        if (r1 == 0)
        {
            semantic_error(program, unary1->right, "reference not found");
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
                    node_class_t *class2 = (node_class_t *)item1->value;
                    node_block_t *block2 = class2->block->value;
                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_FUN)
                        {
                            node_fun_t *fun1 = (node_fun_t *)item2->value;
                            if (semantic_idstrcmp(fun1->key, "+") == 1)
                            {
                                node_t *nps1 = fun1->parameters;
                                node_t *nds2 = NULL;
                                int32_t r2 = semantic_eqaul_psas(program, nps1, nds2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r3 = semantic_resolve(program, NULL, fun1->result, response2, SELECT_FLAG_NONE);
                                    if (r3 == -1)
                                    {
                                        return -1;
                                    }
                                    if (r3 == 0)
                                    {
                                        semantic_error(program, fun1->result, "reference not found");
                                        return -1;
                                    }
                                    else
                                    if (r3 == 1)
                                    {
                                        ilist_t *a3;
                                        for (a3 = response2->begin;a3 != response2->end;a3 = a3->next)
                                        {
                                            node_t *item3 = (node_t *)a3->value;

                                            node_t *clone1 = node_clone(item3->parent, item3);
                                            if (clone1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            
                                            if (item3->kind == NODE_KIND_CLASS)
                                            {
                                                clone1->flag |= NODE_FLAG_NEW;
                                            }

                                            ilist_t *il1 = list_rpush(response, clone1);
                                            if (il1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }
                                    }
                                    list_destroy(response2);
                                }
                            }
                        }
                    }
                }
            }
        }
        list_destroy(response1);
        return 0;
    }
    else
    if (node->kind == NODE_KIND_NEG)
    {
        node_unary_t *unary1 = (node_unary_t *)node->value;

        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_resolve(program, NULL, unary1->right, response1, SELECT_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        if (r1 == 0)
        {
            semantic_error(program, unary1->right, "reference not found");
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
                    node_class_t *class2 = (node_class_t *)item1->value;
                    node_block_t *block2 = class2->block->value;
                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_FUN)
                        {
                            node_fun_t *fun1 = (node_fun_t *)item2->value;
                            if (semantic_idstrcmp(fun1->key, "-") == 1)
                            {
                                node_t *nps1 = fun1->parameters;
                                node_t *nds2 = NULL;
                                int32_t r2 = semantic_eqaul_psas(program, nps1, nds2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r3 = semantic_resolve(program, NULL, fun1->result, response2, SELECT_FLAG_NONE);
                                    if (r3 == -1)
                                    {
                                        return -1;
                                    }
                                    if (r3 == 0)
                                    {
                                        semantic_error(program, fun1->result, "reference not found");
                                        return -1;
                                    }
                                    else
                                    if (r3 == 1)
                                    {
                                        ilist_t *a3;
                                        for (a3 = response2->begin;a3 != response2->end;a3 = a3->next)
                                        {
                                            node_t *item3 = (node_t *)a3->value;

                                            node_t *clone1 = node_clone(item3->parent, item3);
                                            if (clone1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            
                                            if (item3->kind == NODE_KIND_CLASS)
                                            {
                                                clone1->flag |= NODE_FLAG_NEW;
                                            }

                                            ilist_t *il1 = list_rpush(response, clone1);
                                            if (il1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }
                                    }
                                    list_destroy(response2);
                                }
                            }
                        }
                    }
                }
            }
        }
        list_destroy(response1);
        return 0;
    }
    else
    if (node->kind == NODE_KIND_NOT)
    {
        node_unary_t *unary1 = (node_unary_t *)node->value;

        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_resolve(program, NULL, unary1->right, response1, SELECT_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        if (r1 == 0)
        {
            semantic_error(program, unary1->right, "reference not found");
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
                    node_class_t *class2 = (node_class_t *)item1->value;
                    node_block_t *block2 = class2->block->value;
                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_FUN)
                        {
                            node_fun_t *fun1 = (node_fun_t *)item2->value;
                            if (semantic_idstrcmp(fun1->key, "!") == 1)
                            {
                                node_t *nps1 = fun1->parameters;
                                node_t *nds2 = NULL;
                                int32_t r2 = semantic_eqaul_psas(program, nps1, nds2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r3 = semantic_resolve(program, NULL, fun1->result, response2, SELECT_FLAG_NONE);
                                    if (r3 == -1)
                                    {
                                        return -1;
                                    }
                                    if (r3 == 0)
                                    {
                                        semantic_error(program, fun1->result, "reference not found");
                                        return -1;
                                    }
                                    else
                                    if (r3 == 1)
                                    {
                                        ilist_t *a3;
                                        for (a3 = response2->begin;a3 != response2->end;a3 = a3->next)
                                        {
                                            node_t *item3 = (node_t *)a3->value;

                                            node_t *clone1 = node_clone(item3->parent, item3);
                                            if (clone1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            
                                            if (item3->kind == NODE_KIND_CLASS)
                                            {
                                                clone1->flag |= NODE_FLAG_NEW;
                                            }

                                            ilist_t *il1 = list_rpush(response, clone1);
                                            if (il1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }
                                    }
                                    list_destroy(response2);
                                }
                            }
                        }
                    }
                }
            }
        }
        list_destroy(response1);
        return 0;
    }
    else
    if (node->kind == NODE_KIND_TYPEOF)
    {
        node_t *node2 = node_create(node, node->position);
        if (node2 == NULL)
        {
            return -1;
        }
        node_t *node3 = node_make_id(node2, "UInt64");
        if (!node3)
        {
            return -1;
        }
        node3->flag = NODE_FLAG_TEMPORARY;

        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_select(program, node3->parent, NULL, node3, response1, SELECT_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        if (r1 == 0)
        {
            semantic_error(program, node3->parent, "reference not found");
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
                    node_t *clone1 = node_clone(item1->parent, item1);
                    if (clone1 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                    clone1->flag |= NODE_FLAG_NEW;

                    ilist_t *il1 = list_rpush(response, clone1);
                    if (il1 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                    node_prug(node3);
                    list_destroy(response1);
                    return 1;
                }
                else
                if (item1->kind == NODE_KIND_PACKAGE)
                {
                    node_package_t *package1 = (node_package_t *)item1->value;

                    node_t *root1 = item1->parent;
                    while (root1 != NULL)
                    {
                        if (root1->kind == NODE_KIND_IMPORT)
                        {
                            break;
                        }
                        else
                        {
                            root1 = root1->parent;
                        }
                    }

                    if (root1->kind != NODE_KIND_IMPORT)
                    {
                        semantic_error(program, root1, "import not found\n");
                        return -1;
                    }

                    node_import_t *import1 = (node_import_t *)root1->value;

                    node_basic_t *basic2 = (node_basic_t *)import1->path->value;

                    node_t *module1 = program_load(program, basic2->value);
                    if (module1 == NULL)
                    {
                        return -1;
                    }

                    list_t *response2 = list_create();
                    if (response2 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r2 = semantic_postfix(program, module1, package1->route, response2, SELECT_FLAG_NONE);
                    if (r2 == -1)
                    {
                        return -1;
                    }
                    else
                    if (r2 == 0)
                    {
                        semantic_error(program, package1->route, "reference not found");
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
                                node_t *clone2 = node_clone(item2->parent, item2);
                                if (clone2 == NULL)
                                {
                                    return -1;
                                }
                                clone2->flag |= NODE_FLAG_NEW;

                                ilist_t *il1 = list_rpush(response, clone2);
                                if (il1 == NULL)
                                {
                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                            }
                            else
                            {
                                semantic_error(program, item2, "non-class");
                                return -1;
                            }
                            return 1;

                        }
                    }
                    list_destroy(response2);
                }
                else
                {
                    semantic_error(program, item1, "reference not found");
                    return -1;
                }
            }
        }
        node_prug(node3);
        list_destroy(response1);
        return 0;
    }
    else
    if (node->kind == NODE_KIND_SIZEOF)
    {
        node_t *node2 = node_create(node, node->position);
        if (node2 == NULL)
        {
            return -1;
        }
        node_t *node3 = node_make_id(node2, "UInt64");
        if (!node3)
        {
            return -1;
        }
        node3->flag = NODE_FLAG_TEMPORARY;

        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_select(program, node3->parent, NULL, node3, response1, SELECT_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        if (r1 == 0)
        {
            semantic_error(program, node3->parent, "reference not found");
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
                    node_t *clone1 = node_clone(item1->parent, item1);
                    if (clone1 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                    clone1->flag |= NODE_FLAG_NEW;

                    ilist_t *il1 = list_rpush(response, clone1);
                    if (il1 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                    node_prug(node3);
                    list_destroy(response1);
                    return 1;
                }
                else
                if (item1->kind == NODE_KIND_PACKAGE)
                {
                    node_package_t *package1 = (node_package_t *)item1->value;

                    node_t *root1 = item1->parent;
                    while (root1 != NULL)
                    {
                        if (root1->kind == NODE_KIND_IMPORT)
                        {
                            break;
                        }
                        else
                        {
                            root1 = root1->parent;
                        }
                    }

                    if (root1->kind != NODE_KIND_IMPORT)
                    {
                        semantic_error(program, root1, "import not found\n");
                        return -1;
                    }

                    node_import_t *import1 = (node_import_t *)root1->value;

                    node_basic_t *basic2 = (node_basic_t *)import1->path->value;

                    node_t *module1 = program_load(program, basic2->value);
                    if (module1 == NULL)
                    {
                        return -1;
                    }

                    list_t *response2 = list_create();
                    if (response2 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r2 = semantic_postfix(program, module1, package1->route, response2, SELECT_FLAG_NONE);
                    if (r2 == -1)
                    {
                        return -1;
                    }
                    else
                    if (r2 == 0)
                    {
                        semantic_error(program, package1->route, "reference not found");
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
                                node_t *clone2 = node_clone(item2->parent, item2);
                                if (clone2 == NULL)
                                {
                                    return -1;
                                }
                                clone2->flag |= NODE_FLAG_NEW;

                                ilist_t *il1 = list_rpush(response, clone2);
                                if (il1 == NULL)
                                {
                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                            }
                            else
                            {
                                semantic_error(program, item2, "non-class");
                                return -1;
                            }
                            return 1;

                        }
                    }
                    list_destroy(response2);
                }
                else
                {
                    semantic_error(program, item1, "reference not found");
                    return -1;
                }
            }
        }
        node_prug(node3);
        list_destroy(response1);
        return 0;
    }
    else
    {
        return semantic_resolve(program, scope, node, response, flag);
    }
}

static int32_t
semantic_pow(program_t *program, node_t *scope, node_t *node, list_t *response, uint64_t flag)
{
    if (node->kind == NODE_KIND_POW)
    {
        node_binary_t *binary1 = (node_binary_t *)node->value;
        
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_pow(program, NULL, binary1->left, response1, SELECT_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        if (r1 == 0)
        {
            semantic_error(program, binary1->left, "reference not found");
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
                    node_class_t *class2 = (node_class_t *)item1->value;
                    node_block_t *block2 = class2->block->value;
                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_FUN)
                        {
                            node_fun_t *fun1 = (node_fun_t *)item2->value;
                            if (semantic_idstrcmp(fun1->key, "**") == 1)
                            {
                                node_t *data = semantic_make_arguments(program, binary1->right, 1, binary1->right);
                                if (data == NULL)
                                {
                                    return -1;
                                }

                                node_t *nps1 = fun1->parameters;
                                node_t *nds2 = data;
                                int32_t r2 = semantic_eqaul_psas(program, nps1, nds2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r3 = semantic_resolve(program, NULL, fun1->result, response2, SELECT_FLAG_NONE);
                                    if (r3 == -1)
                                    {
                                        return -1;
                                    }
                                    if (r3 == 0)
                                    {
                                        semantic_error(program, fun1->result, "reference not found");
                                        return -1;
                                    }
                                    else
                                    if (r3 == 1)
                                    {
                                        ilist_t *a3;
                                        for (a3 = response2->begin;a3 != response2->end;a3 = a3->next)
                                        {
                                            node_t *item3 = (node_t *)a3->value;

                                            node_t *clone1 = node_clone(item3->parent, item3);
                                            if (clone1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            
                                            if (item3->kind == NODE_KIND_CLASS)
                                            {
                                                clone1->flag |= NODE_FLAG_NEW;
                                            }

                                            ilist_t *il1 = list_rpush(response, clone1);
                                            if (il1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }
                                    }
                                    list_destroy(response2);
                                }
                            }
                        }
                    }
                }
            }
        }
        list_destroy(response1);
        return 0;
        
    }
    else
    {
        return semantic_prefix(program, scope, node, response, flag);
    }
}

static int32_t
semantic_multipicative(program_t *program, node_t *scope, node_t *node, list_t *response, uint64_t flag)
{
    if (node->kind == NODE_KIND_MUL)
    {
        node_binary_t *binary1 = (node_binary_t *)node->value;
        
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_multipicative(program, NULL, binary1->left, response1, SELECT_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        if (r1 == 0)
        {
            semantic_error(program, binary1->left, "reference not found");
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
                    node_class_t *class2 = (node_class_t *)item1->value;
                    node_block_t *block2 = class2->block->value;
                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_FUN)
                        {
                            node_fun_t *fun1 = (node_fun_t *)item2->value;
                            if (semantic_idstrcmp(fun1->key, "*") == 1)
                            {
                                node_t *data = semantic_make_arguments(program, binary1->right, 1, binary1->right);
                                if (data == NULL)
                                {
                                    return -1;
                                }

                                node_t *nps1 = fun1->parameters;
                                node_t *nds2 = data;
                                int32_t r2 = semantic_eqaul_psas(program, nps1, nds2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r3 = semantic_resolve(program, NULL, fun1->result, response2, SELECT_FLAG_NONE);
                                    if (r3 == -1)
                                    {
                                        return -1;
                                    }
                                    if (r3 == 0)
                                    {
                                        semantic_error(program, fun1->result, "reference not found");
                                        return -1;
                                    }
                                    else
                                    if (r3 == 1)
                                    {
                                        ilist_t *a3;
                                        for (a3 = response2->begin;a3 != response2->end;a3 = a3->next)
                                        {
                                            node_t *item3 = (node_t *)a3->value;

                                            node_t *clone1 = node_clone(item3->parent, item3);
                                            if (clone1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            
                                            if (item3->kind == NODE_KIND_CLASS)
                                            {
                                                clone1->flag |= NODE_FLAG_NEW;
                                            }

                                            ilist_t *il1 = list_rpush(response, clone1);
                                            if (il1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }
                                    }
                                    list_destroy(response2);
                                }
                            }
                        }
                    }
                }
            }
        }
        list_destroy(response1);
        return 0; 
    }
    else
    if (node->kind == NODE_KIND_DIV)
    {
        node_binary_t *binary1 = (node_binary_t *)node->value;
        
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_multipicative(program, NULL, binary1->left, response1, SELECT_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        if (r1 == 0)
        {
            semantic_error(program, binary1->left, "reference not found");
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
                    node_class_t *class2 = (node_class_t *)item1->value;
                    node_block_t *block2 = class2->block->value;
                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_FUN)
                        {
                            node_fun_t *fun1 = (node_fun_t *)item2->value;
                            if (semantic_idstrcmp(fun1->key, "/") == 1)
                            {
                                node_t *data = semantic_make_arguments(program, binary1->right, 1, binary1->right);
                                if (data == NULL)
                                {
                                    return -1;
                                }

                                node_t *nps1 = fun1->parameters;
                                node_t *nds2 = data;
                                int32_t r2 = semantic_eqaul_psas(program, nps1, nds2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r3 = semantic_resolve(program, NULL, fun1->result, response2, SELECT_FLAG_NONE);
                                    if (r3 == -1)
                                    {
                                        return -1;
                                    }
                                    if (r3 == 0)
                                    {
                                        semantic_error(program, fun1->result, "reference not found");
                                        return -1;
                                    }
                                    else
                                    if (r3 == 1)
                                    {
                                        ilist_t *a3;
                                        for (a3 = response2->begin;a3 != response2->end;a3 = a3->next)
                                        {
                                            node_t *item3 = (node_t *)a3->value;

                                            node_t *clone1 = node_clone(item3->parent, item3);
                                            if (clone1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            
                                            if (item3->kind == NODE_KIND_CLASS)
                                            {
                                                clone1->flag |= NODE_FLAG_NEW;
                                            }

                                            ilist_t *il1 = list_rpush(response, clone1);
                                            if (il1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }
                                    }
                                    list_destroy(response2);
                                }
                            }
                        }
                    }
                }
            }
        }
        list_destroy(response1);
        return 0; 
    }
    else
    if (node->kind == NODE_KIND_MOD)
    {
        node_binary_t *binary1 = (node_binary_t *)node->value;
        
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_multipicative(program, NULL, binary1->left, response1, SELECT_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        if (r1 == 0)
        {
            semantic_error(program, binary1->left, "reference not found");
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
                    node_class_t *class2 = (node_class_t *)item1->value;
                    node_block_t *block2 = class2->block->value;
                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_FUN)
                        {
                            node_fun_t *fun1 = (node_fun_t *)item2->value;
                            if (semantic_idstrcmp(fun1->key, "%") == 1)
                            {
                                node_t *data = semantic_make_arguments(program, binary1->right, 1, binary1->right);
                                if (data == NULL)
                                {
                                    return -1;
                                }

                                node_t *nps1 = fun1->parameters;
                                node_t *nds2 = data;
                                int32_t r2 = semantic_eqaul_psas(program, nps1, nds2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r3 = semantic_resolve(program, NULL, fun1->result, response2, SELECT_FLAG_NONE);
                                    if (r3 == -1)
                                    {
                                        return -1;
                                    }
                                    if (r3 == 0)
                                    {
                                        semantic_error(program, fun1->result, "reference not found");
                                        return -1;
                                    }
                                    else
                                    if (r3 == 1)
                                    {
                                        ilist_t *a3;
                                        for (a3 = response2->begin;a3 != response2->end;a3 = a3->next)
                                        {
                                            node_t *item3 = (node_t *)a3->value;

                                            node_t *clone1 = node_clone(item3->parent, item3);
                                            if (clone1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            
                                            if (item3->kind == NODE_KIND_CLASS)
                                            {
                                                clone1->flag |= NODE_FLAG_NEW;
                                            }

                                            ilist_t *il1 = list_rpush(response, clone1);
                                            if (il1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }
                                    }
                                    list_destroy(response2);
                                }
                            }
                        }
                    }
                }
            }
        }
        list_destroy(response1);
        return 0; 
    }
    else
    if (node->kind == NODE_KIND_EPI)
    {
        node_binary_t *binary1 = (node_binary_t *)node->value;
        
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_multipicative(program, NULL, binary1->left, response1, SELECT_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        if (r1 == 0)
        {
            semantic_error(program, binary1->left, "reference not found");
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
                    node_class_t *class2 = (node_class_t *)item1->value;
                    node_block_t *block2 = class2->block->value;
                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_FUN)
                        {
                            node_fun_t *fun1 = (node_fun_t *)item2->value;
                            if (semantic_idstrcmp(fun1->key, "\\") == 1)
                            {
                                node_t *data = semantic_make_arguments(program, binary1->right, 1, binary1->right);
                                if (data == NULL)
                                {
                                    return -1;
                                }

                                node_t *nps1 = fun1->parameters;
                                node_t *nds2 = data;
                                int32_t r2 = semantic_eqaul_psas(program, nps1, nds2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r3 = semantic_resolve(program, NULL, fun1->result, response2, SELECT_FLAG_NONE);
                                    if (r3 == -1)
                                    {
                                        return -1;
                                    }
                                    if (r3 == 0)
                                    {
                                        semantic_error(program, fun1->result, "reference not found");
                                        return -1;
                                    }
                                    else
                                    if (r3 == 1)
                                    {
                                        ilist_t *a3;
                                        for (a3 = response2->begin;a3 != response2->end;a3 = a3->next)
                                        {
                                            node_t *item3 = (node_t *)a3->value;

                                            node_t *clone1 = node_clone(item3->parent, item3);
                                            if (clone1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            
                                            if (item3->kind == NODE_KIND_CLASS)
                                            {
                                                clone1->flag |= NODE_FLAG_NEW;
                                            }

                                            ilist_t *il1 = list_rpush(response, clone1);
                                            if (il1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }
                                    }
                                    list_destroy(response2);
                                }
                            }
                        }
                    }
                }
            }
        }
        list_destroy(response1);
        return 0; 
    }
    else
    {
        return semantic_pow(program, scope, node, response, flag);
    }
}

static int32_t
semantic_addative(program_t *program, node_t *scope, node_t *node, list_t *response, uint64_t flag)
{
    if (node->kind == NODE_KIND_PLUS)
    {
        node_binary_t *binary1 = (node_binary_t *)node->value;
        
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_addative(program, NULL, binary1->left, response1, SELECT_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        if (r1 == 0)
        {
            semantic_error(program, binary1->left, "reference not found");
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
                    node_class_t *class2 = (node_class_t *)item1->value;
                    node_block_t *block2 = class2->block->value;
                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_FUN)
                        {
                            node_fun_t *fun1 = (node_fun_t *)item2->value;
                            if (semantic_idstrcmp(fun1->key, "+") == 1)
                            {
                                node_t *data = semantic_make_arguments(program, binary1->right, 1, binary1->right);
                                if (data == NULL)
                                {
                                    return -1;
                                }

                                node_t *nps1 = fun1->parameters;
                                node_t *nds2 = data;
                                int32_t r2 = semantic_eqaul_psas(program, nps1, nds2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r3 = semantic_resolve(program, NULL, fun1->result, response2, SELECT_FLAG_NONE);
                                    if (r3 == -1)
                                    {
                                        return -1;
                                    }
                                    if (r3 == 0)
                                    {
                                        semantic_error(program, fun1->result, "reference not found");
                                        return -1;
                                    }
                                    else
                                    if (r3 == 1)
                                    {
                                        ilist_t *a3;
                                        for (a3 = response2->begin;a3 != response2->end;a3 = a3->next)
                                        {
                                            node_t *item3 = (node_t *)a3->value;

                                            node_t *clone1 = node_clone(item3->parent, item3);
                                            if (clone1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            
                                            if (item3->kind == NODE_KIND_CLASS)
                                            {
                                                clone1->flag |= NODE_FLAG_NEW;
                                            }

                                            ilist_t *il1 = list_rpush(response, clone1);
                                            if (il1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }
                                    }
                                    list_destroy(response2);
                                }
                            }
                        }
                    }
                }
            }
        }
        list_destroy(response1);
        return 0; 
    }
    else
    if (node->kind == NODE_KIND_MINUS)
    {
        node_binary_t *binary1 = (node_binary_t *)node->value;
        
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_addative(program, NULL, binary1->left, response1, SELECT_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        if (r1 == 0)
        {
            semantic_error(program, binary1->left, "reference not found");
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
                    node_class_t *class2 = (node_class_t *)item1->value;
                    node_block_t *block2 = class2->block->value;
                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_FUN)
                        {
                            node_fun_t *fun1 = (node_fun_t *)item2->value;
                            if (semantic_idstrcmp(fun1->key, "-") == 1)
                            {
                                node_t *data = semantic_make_arguments(program, binary1->right, 1, binary1->right);
                                if (data == NULL)
                                {
                                    return -1;
                                }

                                node_t *nps1 = fun1->parameters;
                                node_t *nds2 = data;
                                int32_t r2 = semantic_eqaul_psas(program, nps1, nds2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r3 = semantic_resolve(program, NULL, fun1->result, response2, SELECT_FLAG_NONE);
                                    if (r3 == -1)
                                    {
                                        return -1;
                                    }
                                    if (r3 == 0)
                                    {
                                        semantic_error(program, fun1->result, "reference not found");
                                        return -1;
                                    }
                                    else
                                    if (r3 == 1)
                                    {
                                        ilist_t *a3;
                                        for (a3 = response2->begin;a3 != response2->end;a3 = a3->next)
                                        {
                                            node_t *item3 = (node_t *)a3->value;

                                            node_t *clone1 = node_clone(item3->parent, item3);
                                            if (clone1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            
                                            if (item3->kind == NODE_KIND_CLASS)
                                            {
                                                clone1->flag |= NODE_FLAG_NEW;
                                            }

                                            ilist_t *il1 = list_rpush(response, clone1);
                                            if (il1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }
                                    }
                                    list_destroy(response2);
                                }
                            }
                        }
                    }
                }
            }
        }
        list_destroy(response1);
        return 0; 
    }
    else
    {
        return semantic_multipicative(program, scope, node, response, flag);
    }
}

static int32_t
semantic_shifting(program_t *program, node_t *scope, node_t *node, list_t *response, uint64_t flag)
{
    if (node->kind == NODE_KIND_SHL)
    {
        node_binary_t *binary1 = (node_binary_t *)node->value;
        
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_addative(program, NULL, binary1->left, response1, SELECT_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        if (r1 == 0)
        {
            semantic_error(program, binary1->left, "reference not found");
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
                    node_class_t *class2 = (node_class_t *)item1->value;
                    node_block_t *block2 = class2->block->value;
                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_FUN)
                        {
                            node_fun_t *fun1 = (node_fun_t *)item2->value;
                            if (semantic_idstrcmp(fun1->key, "<<") == 1)
                            {
                                node_t *data = semantic_make_arguments(program, binary1->right, 1, binary1->right);
                                if (data == NULL)
                                {
                                    return -1;
                                }

                                node_t *nps1 = fun1->parameters;
                                node_t *nds2 = data;
                                int32_t r2 = semantic_eqaul_psas(program, nps1, nds2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r3 = semantic_resolve(program, NULL, fun1->result, response2, SELECT_FLAG_NONE);
                                    if (r3 == -1)
                                    {
                                        return -1;
                                    }
                                    if (r3 == 0)
                                    {
                                        semantic_error(program, fun1->result, "reference not found");
                                        return -1;
                                    }
                                    else
                                    if (r3 == 1)
                                    {
                                        ilist_t *a3;
                                        for (a3 = response2->begin;a3 != response2->end;a3 = a3->next)
                                        {
                                            node_t *item3 = (node_t *)a3->value;

                                            node_t *clone1 = node_clone(item3->parent, item3);
                                            if (clone1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            
                                            if (item3->kind == NODE_KIND_CLASS)
                                            {
                                                clone1->flag |= NODE_FLAG_NEW;
                                            }

                                            ilist_t *il1 = list_rpush(response, clone1);
                                            if (il1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }
                                    }
                                    list_destroy(response2);
                                }
                            }
                        }
                    }
                }
            }
        }
        list_destroy(response1);
        return 0; 
    }
    else
    if (node->kind == NODE_KIND_SHR)
    {
        node_binary_t *binary1 = (node_binary_t *)node->value;
        
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_addative(program, NULL, binary1->left, response1, SELECT_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        if (r1 == 0)
        {
            semantic_error(program, binary1->left, "reference not found");
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
                    node_class_t *class2 = (node_class_t *)item1->value;
                    node_block_t *block2 = class2->block->value;
                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_FUN)
                        {
                            node_fun_t *fun1 = (node_fun_t *)item2->value;
                            if (semantic_idstrcmp(fun1->key, ">>") == 1)
                            {
                                node_t *data = semantic_make_arguments(program, binary1->right, 1, binary1->right);
                                if (data == NULL)
                                {
                                    return -1;
                                }

                                node_t *nps1 = fun1->parameters;
                                node_t *nds2 = data;
                                int32_t r2 = semantic_eqaul_psas(program, nps1, nds2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r3 = semantic_resolve(program, NULL, fun1->result, response2, SELECT_FLAG_NONE);
                                    if (r3 == -1)
                                    {
                                        return -1;
                                    }
                                    if (r3 == 0)
                                    {
                                        semantic_error(program, fun1->result, "reference not found");
                                        return -1;
                                    }
                                    else
                                    if (r3 == 1)
                                    {
                                        ilist_t *a3;
                                        for (a3 = response2->begin;a3 != response2->end;a3 = a3->next)
                                        {
                                            node_t *item3 = (node_t *)a3->value;

                                            node_t *clone1 = node_clone(item3->parent, item3);
                                            if (clone1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            
                                            if (item3->kind == NODE_KIND_CLASS)
                                            {
                                                clone1->flag |= NODE_FLAG_NEW;
                                            }

                                            ilist_t *il1 = list_rpush(response, clone1);
                                            if (il1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }
                                    }
                                    list_destroy(response2);
                                }
                            }
                        }
                    }
                }
            }
        }
        list_destroy(response1);
        return 0; 
    }
    else
    {
        return semantic_addative(program, scope, node, response, flag);
    }
}

static int32_t
semantic_relational(program_t *program, node_t *scope, node_t *node, list_t *response, uint64_t flag)
{
    if (node->kind == NODE_KIND_LT)
    {
        node_binary_t *binary1 = (node_binary_t *)node->value;
        
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_shifting(program, NULL, binary1->left, response1, SELECT_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        if (r1 == 0)
        {
            semantic_error(program, binary1->left, "reference not found");
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
                    node_class_t *class2 = (node_class_t *)item1->value;
                    node_block_t *block2 = class2->block->value;
                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_FUN)
                        {
                            node_fun_t *fun1 = (node_fun_t *)item2->value;
                            if (semantic_idstrcmp(fun1->key, "<") == 1)
                            {
                                node_t *data = semantic_make_arguments(program, binary1->right, 1, binary1->right);
                                if (data == NULL)
                                {
                                    return -1;
                                }

                                node_t *nps1 = fun1->parameters;
                                node_t *nds2 = data;
                                int32_t r2 = semantic_eqaul_psas(program, nps1, nds2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r3 = semantic_resolve(program, NULL, fun1->result, response2, SELECT_FLAG_NONE);
                                    if (r3 == -1)
                                    {
                                        return -1;
                                    }
                                    if (r3 == 0)
                                    {
                                        semantic_error(program, fun1->result, "reference not found");
                                        return -1;
                                    }
                                    else
                                    if (r3 == 1)
                                    {
                                        ilist_t *a3;
                                        for (a3 = response2->begin;a3 != response2->end;a3 = a3->next)
                                        {
                                            node_t *item3 = (node_t *)a3->value;

                                            node_t *clone1 = node_clone(item3->parent, item3);
                                            if (clone1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            
                                            if (item3->kind == NODE_KIND_CLASS)
                                            {
                                                clone1->flag |= NODE_FLAG_NEW;
                                            }

                                            ilist_t *il1 = list_rpush(response, clone1);
                                            if (il1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }
                                    }
                                    list_destroy(response2);
                                }
                            }
                        }
                    }
                }
            }
        }
        list_destroy(response1);
        return 0; 
    }
    else
    if (node->kind == NODE_KIND_LE)
    {
        node_binary_t *binary1 = (node_binary_t *)node->value;
        
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_shifting(program, NULL, binary1->left, response1, SELECT_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        if (r1 == 0)
        {
            semantic_error(program, binary1->left, "reference not found");
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
                    node_class_t *class2 = (node_class_t *)item1->value;
                    node_block_t *block2 = class2->block->value;
                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_FUN)
                        {
                            node_fun_t *fun1 = (node_fun_t *)item2->value;
                            if (semantic_idstrcmp(fun1->key, "<=") == 1)
                            {
                                node_t *data = semantic_make_arguments(program, binary1->right, 1, binary1->right);
                                if (data == NULL)
                                {
                                    return -1;
                                }

                                node_t *nps1 = fun1->parameters;
                                node_t *nds2 = data;
                                int32_t r2 = semantic_eqaul_psas(program, nps1, nds2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r3 = semantic_resolve(program, NULL, fun1->result, response2, SELECT_FLAG_NONE);
                                    if (r3 == -1)
                                    {
                                        return -1;
                                    }
                                    if (r3 == 0)
                                    {
                                        semantic_error(program, fun1->result, "reference not found");
                                        return -1;
                                    }
                                    else
                                    if (r3 == 1)
                                    {
                                        ilist_t *a3;
                                        for (a3 = response2->begin;a3 != response2->end;a3 = a3->next)
                                        {
                                            node_t *item3 = (node_t *)a3->value;

                                            node_t *clone1 = node_clone(item3->parent, item3);
                                            if (clone1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            
                                            if (item3->kind == NODE_KIND_CLASS)
                                            {
                                                clone1->flag |= NODE_FLAG_NEW;
                                            }

                                            ilist_t *il1 = list_rpush(response, clone1);
                                            if (il1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }
                                    }
                                    list_destroy(response2);
                                }
                            }
                        }
                    }
                }
            }
        }
        list_destroy(response1);
        return 0; 
    }
    else
    if (node->kind == NODE_KIND_GT)
    {
        node_binary_t *binary1 = (node_binary_t *)node->value;
        
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_shifting(program, NULL, binary1->left, response1, SELECT_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        if (r1 == 0)
        {
            semantic_error(program, binary1->left, "reference not found");
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
                    node_class_t *class2 = (node_class_t *)item1->value;
                    node_block_t *block2 = class2->block->value;
                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_FUN)
                        {
                            node_fun_t *fun1 = (node_fun_t *)item2->value;
                            if (semantic_idstrcmp(fun1->key, ">") == 1)
                            {
                                node_t *data = semantic_make_arguments(program, binary1->right, 1, binary1->right);
                                if (data == NULL)
                                {
                                    return -1;
                                }

                                node_t *nps1 = fun1->parameters;
                                node_t *nds2 = data;
                                int32_t r2 = semantic_eqaul_psas(program, nps1, nds2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r3 = semantic_resolve(program, NULL, fun1->result, response2, SELECT_FLAG_NONE);
                                    if (r3 == -1)
                                    {
                                        return -1;
                                    }
                                    if (r3 == 0)
                                    {
                                        semantic_error(program, fun1->result, "reference not found");
                                        return -1;
                                    }
                                    else
                                    if (r3 == 1)
                                    {
                                        ilist_t *a3;
                                        for (a3 = response2->begin;a3 != response2->end;a3 = a3->next)
                                        {
                                            node_t *item3 = (node_t *)a3->value;

                                            node_t *clone1 = node_clone(item3->parent, item3);
                                            if (clone1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            
                                            if (item3->kind == NODE_KIND_CLASS)
                                            {
                                                clone1->flag |= NODE_FLAG_NEW;
                                            }

                                            ilist_t *il1 = list_rpush(response, clone1);
                                            if (il1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }
                                    }
                                    list_destroy(response2);
                                }
                            }
                        }
                    }
                }
            }
        }
        list_destroy(response1);
        return 0; 
    }
    else
    if (node->kind == NODE_KIND_GE)
    {
        node_binary_t *binary1 = (node_binary_t *)node->value;
        
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_shifting(program, NULL, binary1->left, response1, SELECT_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        if (r1 == 0)
        {
            semantic_error(program, binary1->left, "reference not found");
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
                    node_class_t *class2 = (node_class_t *)item1->value;
                    node_block_t *block2 = class2->block->value;
                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_FUN)
                        {
                            node_fun_t *fun1 = (node_fun_t *)item2->value;
                            if (semantic_idstrcmp(fun1->key, ">=") == 1)
                            {
                                node_t *data = semantic_make_arguments(program, binary1->right, 1, binary1->right);
                                if (data == NULL)
                                {
                                    return -1;
                                }

                                node_t *nps1 = fun1->parameters;
                                node_t *nds2 = data;
                                int32_t r2 = semantic_eqaul_psas(program, nps1, nds2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r3 = semantic_resolve(program, NULL, fun1->result, response2, SELECT_FLAG_NONE);
                                    if (r3 == -1)
                                    {
                                        return -1;
                                    }
                                    if (r3 == 0)
                                    {
                                        semantic_error(program, fun1->result, "reference not found");
                                        return -1;
                                    }
                                    else
                                    if (r3 == 1)
                                    {
                                        ilist_t *a3;
                                        for (a3 = response2->begin;a3 != response2->end;a3 = a3->next)
                                        {
                                            node_t *item3 = (node_t *)a3->value;

                                            node_t *clone1 = node_clone(item3->parent, item3);
                                            if (clone1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            
                                            if (item3->kind == NODE_KIND_CLASS)
                                            {
                                                clone1->flag |= NODE_FLAG_NEW;
                                            }

                                            ilist_t *il1 = list_rpush(response, clone1);
                                            if (il1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }
                                    }
                                    list_destroy(response2);
                                }
                            }
                        }
                    }
                }
            }
        }
        list_destroy(response1);
        return 0; 
    }
    else
    {
        return semantic_shifting(program, scope, node, response, flag);
    }
}

static int32_t
semantic_equality(program_t *program, node_t *scope, node_t *node, list_t *response, uint64_t flag)
{
    if (node->kind == NODE_KIND_EQ)
    {
        node_binary_t *binary1 = (node_binary_t *)node->value;
        
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_relational(program, NULL, binary1->left, response1, SELECT_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        if (r1 == 0)
        {
            semantic_error(program, binary1->left, "reference not found");
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
                    node_class_t *class2 = (node_class_t *)item1->value;
                    node_block_t *block2 = class2->block->value;
                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_FUN)
                        {
                            node_fun_t *fun1 = (node_fun_t *)item2->value;
                            if (semantic_idstrcmp(fun1->key, "==") == 1)
                            {
                                node_t *data = semantic_make_arguments(program, binary1->right, 1, binary1->right);
                                if (data == NULL)
                                {
                                    return -1;
                                }

                                node_t *nps1 = fun1->parameters;
                                node_t *nds2 = data;
                                int32_t r2 = semantic_eqaul_psas(program, nps1, nds2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r3 = semantic_resolve(program, NULL, fun1->result, response2, SELECT_FLAG_NONE);
                                    if (r3 == -1)
                                    {
                                        return -1;
                                    }
                                    if (r3 == 0)
                                    {
                                        semantic_error(program, fun1->result, "reference not found");
                                        return -1;
                                    }
                                    else
                                    if (r3 == 1)
                                    {
                                        ilist_t *a3;
                                        for (a3 = response2->begin;a3 != response2->end;a3 = a3->next)
                                        {
                                            node_t *item3 = (node_t *)a3->value;

                                            node_t *clone1 = node_clone(item3->parent, item3);
                                            if (clone1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            
                                            if (item3->kind == NODE_KIND_CLASS)
                                            {
                                                clone1->flag |= NODE_FLAG_NEW;
                                            }

                                            ilist_t *il1 = list_rpush(response, clone1);
                                            if (il1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }
                                    }
                                    list_destroy(response2);
                                }
                            }
                        }
                    }
                }
            }
        }
        list_destroy(response1);
        return 0; 
    }
    else
    if (node->kind == NODE_KIND_NEQ)
    {
        node_binary_t *binary1 = (node_binary_t *)node->value;
        
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_relational(program, NULL, binary1->left, response1, SELECT_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        if (r1 == 0)
        {
            semantic_error(program, binary1->left, "reference not found");
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
                    node_class_t *class2 = (node_class_t *)item1->value;
                    node_block_t *block2 = class2->block->value;
                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_FUN)
                        {
                            node_fun_t *fun1 = (node_fun_t *)item2->value;
                            if (semantic_idstrcmp(fun1->key, "!=") == 1)
                            {
                                node_t *data = semantic_make_arguments(program, binary1->right, 1, binary1->right);
                                if (data == NULL)
                                {
                                    return -1;
                                }

                                node_t *nps1 = fun1->parameters;
                                node_t *nds2 = data;
                                int32_t r2 = semantic_eqaul_psas(program, nps1, nds2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r3 = semantic_resolve(program, NULL, fun1->result, response2, SELECT_FLAG_NONE);
                                    if (r3 == -1)
                                    {
                                        return -1;
                                    }
                                    if (r3 == 0)
                                    {
                                        semantic_error(program, fun1->result, "reference not found");
                                        return -1;
                                    }
                                    else
                                    if (r3 == 1)
                                    {
                                        ilist_t *a3;
                                        for (a3 = response2->begin;a3 != response2->end;a3 = a3->next)
                                        {
                                            node_t *item3 = (node_t *)a3->value;

                                            node_t *clone1 = node_clone(item3->parent, item3);
                                            if (clone1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            
                                            if (item3->kind == NODE_KIND_CLASS)
                                            {
                                                clone1->flag |= NODE_FLAG_NEW;
                                            }

                                            ilist_t *il1 = list_rpush(response, clone1);
                                            if (il1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }
                                    }
                                    list_destroy(response2);
                                }
                            }
                        }
                    }
                }
            }
        }
        list_destroy(response1);
        return 0; 
    }
    else
    {
        return semantic_relational(program, scope, node, response, flag);
    }
}

static int32_t
semantic_bitwise_and(program_t *program, node_t *scope, node_t *node, list_t *response, uint64_t flag)
{
    if (node->kind == NODE_KIND_AND)
    {
        node_binary_t *binary1 = (node_binary_t *)node->value;
        
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_bitwise_and(program, NULL, binary1->left, response1, SELECT_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        if (r1 == 0)
        {
            semantic_error(program, binary1->left, "reference not found");
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
                    node_class_t *class2 = (node_class_t *)item1->value;
                    node_block_t *block2 = class2->block->value;
                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_FUN)
                        {
                            node_fun_t *fun1 = (node_fun_t *)item2->value;
                            if (semantic_idstrcmp(fun1->key, "&") == 1)
                            {
                                node_t *data = semantic_make_arguments(program, binary1->right, 1, binary1->right);
                                if (data == NULL)
                                {
                                    return -1;
                                }

                                node_t *nps1 = fun1->parameters;
                                node_t *nds2 = data;
                                int32_t r2 = semantic_eqaul_psas(program, nps1, nds2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r3 = semantic_resolve(program, NULL, fun1->result, response2, SELECT_FLAG_NONE);
                                    if (r3 == -1)
                                    {
                                        return -1;
                                    }
                                    if (r3 == 0)
                                    {
                                        semantic_error(program, fun1->result, "reference not found");
                                        return -1;
                                    }
                                    else
                                    if (r3 == 1)
                                    {
                                        ilist_t *a3;
                                        for (a3 = response2->begin;a3 != response2->end;a3 = a3->next)
                                        {
                                            node_t *item3 = (node_t *)a3->value;

                                            node_t *clone1 = node_clone(item3->parent, item3);
                                            if (clone1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            
                                            if (item3->kind == NODE_KIND_CLASS)
                                            {
                                                clone1->flag |= NODE_FLAG_NEW;
                                            }

                                            ilist_t *il1 = list_rpush(response, clone1);
                                            if (il1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }
                                    }
                                    list_destroy(response2);
                                }
                            }
                        }
                    }
                }
            }
        }
        list_destroy(response1);
        return 0; 
    }
    else
    {
        return semantic_equality(program, scope, node, response, flag);
    }
}

static int32_t
semantic_bitwise_xor(program_t *program, node_t *scope, node_t *node, list_t *response, uint64_t flag)
{
    if (node->kind == NODE_KIND_XOR)
    {
        node_binary_t *binary1 = (node_binary_t *)node->value;
        
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_bitwise_xor(program, NULL, binary1->left, response1, SELECT_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        if (r1 == 0)
        {
            semantic_error(program, binary1->left, "reference not found");
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
                    node_class_t *class2 = (node_class_t *)item1->value;
                    node_block_t *block2 = class2->block->value;
                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_FUN)
                        {
                            node_fun_t *fun1 = (node_fun_t *)item2->value;
                            if (semantic_idstrcmp(fun1->key, "^") == 1)
                            {
                                node_t *data = semantic_make_arguments(program, binary1->right, 1, binary1->right);
                                if (data == NULL)
                                {
                                    return -1;
                                }

                                node_t *nps1 = fun1->parameters;
                                node_t *nds2 = data;
                                int32_t r2 = semantic_eqaul_psas(program, nps1, nds2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r3 = semantic_resolve(program, NULL, fun1->result, response2, SELECT_FLAG_NONE);
                                    if (r3 == -1)
                                    {
                                        return -1;
                                    }
                                    if (r3 == 0)
                                    {
                                        semantic_error(program, fun1->result, "reference not found");
                                        return -1;
                                    }
                                    else
                                    if (r3 == 1)
                                    {
                                        ilist_t *a3;
                                        for (a3 = response2->begin;a3 != response2->end;a3 = a3->next)
                                        {
                                            node_t *item3 = (node_t *)a3->value;

                                            node_t *clone1 = node_clone(item3->parent, item3);
                                            if (clone1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            
                                            if (item3->kind == NODE_KIND_CLASS)
                                            {
                                                clone1->flag |= NODE_FLAG_NEW;
                                            }

                                            ilist_t *il1 = list_rpush(response, clone1);
                                            if (il1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }
                                    }
                                    list_destroy(response2);
                                }
                            }
                        }
                    }
                }
            }
        }
        list_destroy(response1);
        return 0; 
    }
    else
    {
        return semantic_bitwise_and(program, scope, node, response, flag);
    }
}

static int32_t
semantic_bitwise_or(program_t *program, node_t *scope, node_t *node, list_t *response, uint64_t flag)
{
    if (node->kind == NODE_KIND_XOR)
    {
        node_binary_t *binary1 = (node_binary_t *)node->value;
        
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_bitwise_or(program, NULL, binary1->left, response1, SELECT_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        if (r1 == 0)
        {
            semantic_error(program, binary1->left, "reference not found");
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
                    node_class_t *class2 = (node_class_t *)item1->value;
                    node_block_t *block2 = class2->block->value;
                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_FUN)
                        {
                            node_fun_t *fun1 = (node_fun_t *)item2->value;
                            if (semantic_idstrcmp(fun1->key, "|") == 1)
                            {
                                node_t *data = semantic_make_arguments(program, binary1->right, 1, binary1->right);
                                if (data == NULL)
                                {
                                    return -1;
                                }

                                node_t *nps1 = fun1->parameters;
                                node_t *nds2 = data;
                                int32_t r2 = semantic_eqaul_psas(program, nps1, nds2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r3 = semantic_resolve(program, NULL, fun1->result, response2, SELECT_FLAG_NONE);
                                    if (r3 == -1)
                                    {
                                        return -1;
                                    }
                                    if (r3 == 0)
                                    {
                                        semantic_error(program, fun1->result, "reference not found");
                                        return -1;
                                    }
                                    else
                                    if (r3 == 1)
                                    {
                                        ilist_t *a3;
                                        for (a3 = response2->begin;a3 != response2->end;a3 = a3->next)
                                        {
                                            node_t *item3 = (node_t *)a3->value;

                                            node_t *clone1 = node_clone(item3->parent, item3);
                                            if (clone1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            
                                            if (item3->kind == NODE_KIND_CLASS)
                                            {
                                                clone1->flag |= NODE_FLAG_NEW;
                                            }

                                            ilist_t *il1 = list_rpush(response, clone1);
                                            if (il1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }
                                    }
                                    list_destroy(response2);
                                }
                            }
                        }
                    }
                }
            }
        }
        list_destroy(response1);
        return 0; 
    }
    else
    {
        return semantic_bitwise_xor(program, scope, node, response, flag);
    }
}

static int32_t
semantic_logical_and(program_t *program, node_t *scope, node_t *node, list_t *response, uint64_t flag)
{
    if (node->kind == NODE_KIND_LAND)
    {
        node_binary_t *binary1 = (node_binary_t *)node->value;
        
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_logical_and(program, NULL, binary1->left, response1, SELECT_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        if (r1 == 0)
        {
            semantic_error(program, binary1->left, "reference not found");
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
                    node_class_t *class2 = (node_class_t *)item1->value;
                    node_block_t *block2 = class2->block->value;
                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_FUN)
                        {
                            node_fun_t *fun1 = (node_fun_t *)item2->value;
                            if (semantic_idstrcmp(fun1->key, "&&") == 1)
                            {
                                node_t *data = semantic_make_arguments(program, binary1->right, 1, binary1->right);
                                if (data == NULL)
                                {
                                    return -1;
                                }

                                node_t *nps1 = fun1->parameters;
                                node_t *nds2 = data;
                                int32_t r2 = semantic_eqaul_psas(program, nps1, nds2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r3 = semantic_resolve(program, NULL, fun1->result, response2, SELECT_FLAG_NONE);
                                    if (r3 == -1)
                                    {
                                        return -1;
                                    }
                                    if (r3 == 0)
                                    {
                                        semantic_error(program, fun1->result, "reference not found");
                                        return -1;
                                    }
                                    else
                                    if (r3 == 1)
                                    {
                                        ilist_t *a3;
                                        for (a3 = response2->begin;a3 != response2->end;a3 = a3->next)
                                        {
                                            node_t *item3 = (node_t *)a3->value;

                                            node_t *clone1 = node_clone(item3->parent, item3);
                                            if (clone1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            
                                            if (item3->kind == NODE_KIND_CLASS)
                                            {
                                                clone1->flag |= NODE_FLAG_NEW;
                                            }

                                            ilist_t *il1 = list_rpush(response, clone1);
                                            if (il1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }
                                    }
                                    list_destroy(response2);
                                }
                            }
                        }
                    }
                }
            }
        }
        list_destroy(response1);
        return 0; 
    }
    else
    {
        return semantic_bitwise_or(program, scope, node, response, flag);
    }
}

static int32_t
semantic_logical_or(program_t *program, node_t *scope, node_t *node, list_t *response, uint64_t flag)
{
    if (node->kind == NODE_KIND_LOR)
    {
        node_binary_t *binary1 = (node_binary_t *)node->value;
        
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_logical_and(program, NULL, binary1->left, response1, SELECT_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        if (r1 == 0)
        {
            semantic_error(program, binary1->left, "reference not found");
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
                    node_class_t *class2 = (node_class_t *)item1->value;
                    node_block_t *block2 = class2->block->value;
                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_FUN)
                        {
                            node_fun_t *fun1 = (node_fun_t *)item2->value;
                            if (semantic_idstrcmp(fun1->key, "||") == 1)
                            {
                                node_t *data = semantic_make_arguments(program, binary1->right, 1, binary1->right);
                                if (data == NULL)
                                {
                                    return -1;
                                }

                                node_t *nps1 = fun1->parameters;
                                node_t *nds2 = data;
                                int32_t r2 = semantic_eqaul_psas(program, nps1, nds2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r3 = semantic_resolve(program, NULL, fun1->result, response2, SELECT_FLAG_NONE);
                                    if (r3 == -1)
                                    {
                                        return -1;
                                    }
                                    if (r3 == 0)
                                    {
                                        semantic_error(program, fun1->result, "reference not found");
                                        return -1;
                                    }
                                    else
                                    if (r3 == 1)
                                    {
                                        ilist_t *a3;
                                        for (a3 = response2->begin;a3 != response2->end;a3 = a3->next)
                                        {
                                            node_t *item3 = (node_t *)a3->value;

                                            node_t *clone1 = node_clone(item3->parent, item3);
                                            if (clone1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            
                                            if (item3->kind == NODE_KIND_CLASS)
                                            {
                                                clone1->flag |= NODE_FLAG_NEW;
                                            }

                                            ilist_t *il1 = list_rpush(response, clone1);
                                            if (il1 == NULL)
                                            {
                                                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }
                                    }
                                    list_destroy(response2);
                                }
                            }
                        }
                    }
                }
            }
        }
        list_destroy(response1);
        return 0; 
    }
    else
    {
        return semantic_bitwise_or(program, scope, node, response, flag);
    }
}

static int32_t
semantic_expression(program_t *program, node_t *scope, node_t *node, list_t *response, uint64_t flag)
{
	return semantic_logical_or(program, scope, node, response, flag);
}

static int32_t
semantic_assign(program_t *program, node_t *node)
{
    if (node->kind == NODE_KIND_ASSIGN)
    {
        node_binary_t *binary1 = (node_binary_t *)node->value;
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_expression(program, NULL, binary1->left, response1, SELECT_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        if (r1 == 0)
        {
            semantic_error(program, binary1->left, "reference not found");
            return -1;
        }
        else
        if (r1 == 1)
        {
            ilist_t *a1;
            for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                list_t *response2 = list_create();
                if (response2 == NULL)
                {
                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                    return -1;
                }

                int32_t r2 = semantic_expression(program, NULL, binary1->right, response2, SELECT_FLAG_NONE);
                if (r2 == -1)
                {
                    return -1;
                }
                if (r2 == 0)
                {
                    semantic_error(program, binary1->right, "reference not found");
                    return -1;
                }
                else
                if (r2 == 1)
                {
                    ilist_t *a2;
                    for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item1->kind == NODE_KIND_VAR)
                        {
                            node_var_t *var1 = (node_var_t *)item1->value;
                            if (item2->kind == NODE_KIND_VAR)
                            {
                                node_var_t *var2 = (node_var_t *)item2->value;
                                if (var2->value_update != NULL)
                                {
                                    var1->value_update = var2->value_update;
                                    list_destroy(response2);
                                    list_destroy(response1);
                                    return 1;
                                }
                                else
                                {
                                    semantic_error(program, item2, "unitialized, encounter in (%lld:%lld)",
                                        binary1->right->position.line, binary1->right->position.column);
                                    return -1;
                                }
                            }
                            else
                            if (item2->kind == NODE_KIND_ENTITY)
                            {
                                node_entity_t *entity2 = (node_entity_t *)item2->value;
                                if (entity2->value_update != NULL)
                                {
                                    var1->value_update = entity2->value_update;
                                    list_destroy(response2);
                                    list_destroy(response1);
                                    return 1;
                                }
                                else
                                {
                                    semantic_error(program, item2, "unitialized, encounter in (%lld:%lld)",
                                        binary1->right->position.line, binary1->right->position.column);
                                    return -1;
                                }
                            }
                            else
                            if (item2->kind == NODE_KIND_PROPERTY)
                            {
                                node_property_t *property2 = (node_property_t *)item2->value;
                                if (property2->value_update != NULL)
                                {
                                    var1->value_update = property2->value_update;
                                    list_destroy(response2);
                                    list_destroy(response1);
                                    return 1;
                                }
                                else
                                {
                                    semantic_error(program, item2, "unitialized, encounter in (%lld:%lld)",
                                        binary1->right->position.line, binary1->right->position.column);
                                    return -1;
                                }
                            }
                            else
                            if (item2->kind == NODE_KIND_PARAMETER)
                            {
                                node_parameter_t *parameter2 = (node_parameter_t *)item2->value;
                                if (parameter2->value_update != NULL)
                                {
                                    var1->value_update = parameter2->value_update;
                                    list_destroy(response2);
                                    list_destroy(response1);
                                    return 1;
                                }
                                else
                                {
                                    semantic_error(program, item2, "unitialized, encounter in (%lld:%lld)",
                                        binary1->right->position.line, binary1->right->position.column);
                                    return -1;
                                }
                            }
                            else
                            if (item2->kind == NODE_KIND_MEMBER)
                            {
                                node_member_t *member2 = (node_member_t *)item2->value;
                                if (member2->value_update != NULL)
                                {
                                    var1->value_update = member2->value_update;
                                    list_destroy(response2);
                                    list_destroy(response1);
                                    return 1;
                                }
                                else
                                {
                                    semantic_error(program, item2, "unitialized, encounter in (%lld:%lld)",
                                        binary1->right->position.line, binary1->right->position.column);
                                    return -1;
                                }
                            }
                            else
                            {
                                var1->value_update = item2;
                                list_destroy(response2);
                                list_destroy(response1);
                                return 1;
                            }
                        }
                        else
                        if (item1->kind == NODE_KIND_ENTITY)
                        {
                            node_entity_t *entity1 = (node_entity_t *)item1->value;
                            if (item2->kind == NODE_KIND_VAR)
                            {
                                node_var_t *var2 = (node_var_t *)item2->value;
                                if (var2->value_update != NULL)
                                {
                                    entity1->value_update = var2->value_update;
                                    list_destroy(response2);
                                    list_destroy(response1);
                                    return 1;
                                }
                                else
                                {
                                    semantic_error(program, item2, "unitialized, encounter in (%lld:%lld)",
                                        binary1->right->position.line, binary1->right->position.column);
                                    return -1;
                                }
                            }
                            else
                            if (item2->kind == NODE_KIND_ENTITY)
                            {
                                node_entity_t *entity2 = (node_entity_t *)item2->value;
                                if (entity2->value_update != NULL)
                                {
                                    entity1->value_update = entity2->value_update;
                                    list_destroy(response2);
                                    list_destroy(response1);
                                    return 1;
                                }
                                else
                                {
                                    semantic_error(program, item2, "unitialized, encounter in (%lld:%lld)",
                                        binary1->right->position.line, binary1->right->position.column);
                                    return -1;
                                }
                            }
                            else
                            if (item2->kind == NODE_KIND_PROPERTY)
                            {
                                node_property_t *property2 = (node_property_t *)item2->value;
                                if (property2->value_update != NULL)
                                {
                                    entity1->value_update = property2->value_update;
                                    list_destroy(response2);
                                    list_destroy(response1);
                                    return 1;
                                }
                                else
                                {
                                    semantic_error(program, item2, "unitialized, encounter in (%lld:%lld)",
                                        binary1->right->position.line, binary1->right->position.column);
                                    return -1;
                                }
                            }
                            else
                            if (item2->kind == NODE_KIND_PARAMETER)
                            {
                                node_parameter_t *parameter2 = (node_parameter_t *)item2->value;
                                if (parameter2->value_update != NULL)
                                {
                                    entity1->value_update = parameter2->value_update;
                                    list_destroy(response2);
                                    list_destroy(response1);
                                    return 1;
                                }
                                else
                                {
                                    semantic_error(program, item2, "unitialized, encounter in (%lld:%lld)",
                                        binary1->right->position.line, binary1->right->position.column);
                                    return -1;
                                }
                            }
                            else
                            if (item2->kind == NODE_KIND_MEMBER)
                            {
                                node_member_t *member2 = (node_member_t *)item2->value;
                                if (member2->value_update != NULL)
                                {
                                    entity1->value_update = member2->value_update;
                                    list_destroy(response2);
                                    list_destroy(response1);
                                    return 1;
                                }
                                else
                                {
                                    semantic_error(program, item2, "unitialized, encounter in (%lld:%lld)",
                                        binary1->right->position.line, binary1->right->position.column);
                                    return -1;
                                }
                            }
                            else
                            {
                                entity1->value_update = item2;
                                list_destroy(response2);
                                list_destroy(response1);
                                return 1;
                            }
                        }
                        else
                        if (item1->kind == NODE_KIND_PROPERTY)
                        {
                            node_property_t *property1 = (node_property_t *)item1->value;
                            if (item2->kind == NODE_KIND_VAR)
                            {
                                node_var_t *var2 = (node_var_t *)item2->value;
                                if (var2->value_update != NULL)
                                {
                                    property1->value_update = var2->value_update;
                                    list_destroy(response2);
                                    list_destroy(response1);
                                    return 1;
                                }
                                else
                                {
                                    semantic_error(program, item2, "unitialized, encounter in (%lld:%lld)",
                                        binary1->right->position.line, binary1->right->position.column);
                                    return -1;
                                }
                            }
                            else
                            if (item2->kind == NODE_KIND_ENTITY)
                            {
                                node_entity_t *entity2 = (node_entity_t *)item2->value;
                                if (entity2->value_update != NULL)
                                {
                                    property1->value_update = entity2->value_update;
                                    list_destroy(response2);
                                    list_destroy(response1);
                                    return 1;
                                }
                                else
                                {
                                    semantic_error(program, item2, "unitialized, encounter in (%lld:%lld)",
                                        binary1->right->position.line, binary1->right->position.column);
                                    return -1;
                                }
                            }
                            else
                            if (item2->kind == NODE_KIND_PROPERTY)
                            {
                                node_property_t *property2 = (node_property_t *)item2->value;
                                if (property2->value_update != NULL)
                                {
                                    property1->value_update = property2->value_update;
                                    list_destroy(response2);
                                    list_destroy(response1);
                                    return 1;
                                }
                                else
                                {
                                    semantic_error(program, item2, "unitialized, encounter in (%lld:%lld)",
                                        binary1->right->position.line, binary1->right->position.column);
                                    return -1;
                                }
                            }
                            else
                            if (item2->kind == NODE_KIND_PARAMETER)
                            {
                                node_parameter_t *parameter2 = (node_parameter_t *)item2->value;
                                if (parameter2->value_update != NULL)
                                {
                                    property1->value_update = parameter2->value_update;
                                    list_destroy(response2);
                                    list_destroy(response1);
                                    return 1;
                                }
                                else
                                {
                                    semantic_error(program, item2, "unitialized, encounter in (%lld:%lld)",
                                        binary1->right->position.line, binary1->right->position.column);
                                    return -1;
                                }
                            }
                            else
                            if (item2->kind == NODE_KIND_MEMBER)
                            {
                                node_member_t *member2 = (node_member_t *)item2->value;
                                if (member2->value_update != NULL)
                                {
                                    property1->value_update = member2->value_update;
                                    list_destroy(response2);
                                    list_destroy(response1);
                                    return 1;
                                }
                                else
                                {
                                    semantic_error(program, item2, "unitialized, encounter in (%lld:%lld)",
                                        binary1->right->position.line, binary1->right->position.column);
                                    return -1;
                                }
                            }
                            else
                            {
                                property1->value_update = item2;
                                list_destroy(response2);
                                list_destroy(response1);
                                return 1;
                            }
                        }
                        else
                        if (item1->kind == NODE_KIND_PARAMETER)
                        {
                            node_parameter_t *parameter1 = (node_parameter_t *)item1->value;
                            if (item2->kind == NODE_KIND_VAR)
                            {
                                node_var_t *var2 = (node_var_t *)item2->value;
                                if (var2->value_update != NULL)
                                {
                                    parameter1->value_update = var2->value_update;
                                    list_destroy(response2);
                                    list_destroy(response1);
                                    return 1;
                                }
                                else
                                {
                                    semantic_error(program, item2, "unitialized, encounter in (%lld:%lld)",
                                        binary1->right->position.line, binary1->right->position.column);
                                    return -1;
                                }
                            }
                            else
                            if (item2->kind == NODE_KIND_ENTITY)
                            {
                                node_entity_t *entity2 = (node_entity_t *)item2->value;
                                if (entity2->value_update != NULL)
                                {
                                    parameter1->value_update = entity2->value_update;
                                    list_destroy(response2);
                                    list_destroy(response1);
                                    return 1;
                                }
                                else
                                {
                                    semantic_error(program, item2, "unitialized, encounter in (%lld:%lld)",
                                        binary1->right->position.line, binary1->right->position.column);
                                    return -1;
                                }
                            }
                            else
                            if (item2->kind == NODE_KIND_PROPERTY)
                            {
                                node_property_t *property2 = (node_property_t *)item2->value;
                                if (property2->value_update != NULL)
                                {
                                    parameter1->value_update = property2->value_update;
                                    list_destroy(response2);
                                    list_destroy(response1);
                                    return 1;
                                }
                                else
                                {
                                    semantic_error(program, item2, "unitialized, encounter in (%lld:%lld)",
                                        binary1->right->position.line, binary1->right->position.column);
                                    return -1;
                                }
                            }
                            else
                            if (item2->kind == NODE_KIND_PARAMETER)
                            {
                                node_parameter_t *parameter2 = (node_parameter_t *)item2->value;
                                if (parameter2->value_update != NULL)
                                {
                                    parameter1->value_update = parameter2->value_update;
                                    list_destroy(response2);
                                    list_destroy(response1);
                                    return 1;
                                }
                                else
                                {
                                    semantic_error(program, item2, "unitialized, encounter in (%lld:%lld)",
                                        binary1->right->position.line, binary1->right->position.column);
                                    return -1;
                                }
                            }
                            else
                            if (item2->kind == NODE_KIND_MEMBER)
                            {
                                node_member_t *member2 = (node_member_t *)item2->value;
                                if (member2->value_update != NULL)
                                {
                                    parameter1->value_update = member2->value_update;
                                    list_destroy(response2);
                                    list_destroy(response1);
                                    return 1;
                                }
                                else
                                {
                                    semantic_error(program, item2, "unitialized, encounter in (%lld:%lld)",
                                        binary1->right->position.line, binary1->right->position.column);
                                    return -1;
                                }
                            }
                            else
                            {
                                parameter1->value_update = item2;
                                list_destroy(response2);
                                list_destroy(response1);
                                return 1;
                            }
                        }
                        else
                        {
                            semantic_error(program, item2, "incorrect, encounter in (%lld:%lld)",
                                binary1->right->position.line, binary1->right->position.column);
                            return -1;
                        }
                    }
                }
                list_destroy(response2);
            }
        }
        list_destroy(response1);
        return 0;
    }
    else
    {
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_expression(program, NULL, node, response1, SELECT_FLAG_NONE);
        if (r1 == -1)
        {
            list_destroy(response1);
            return -1;
        }
        if (r1 == 0)
        {
            semantic_error(program, node, "reference not found");
            return -1;
        }
        else
        if (r1 == 1)
        {
            list_destroy(response1);
            return 1;
        }
        return 0;
    }
}

static int32_t
semantic_field(program_t *program, node_t *node)
{
    node_field_t *field1 = (node_field_t *)node->value;
    
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
							semantic_error(program, field1->key, "not an key id");
							return -1;
						}
						node_field_t *field3 = (node_field_t *)item2->value;
						if (field3->value != NULL)
						{
							if (field3->key->kind != NODE_KIND_ID)
							{
								semantic_error(program, field3->key, "not an key id");
								return -1;
							}
							if (semantic_idcmp(field1->key, field3->key) == 1)
							{
								semantic_error(program, field1->key, "already defined, previous in (%lld:%lld)",
									field3->key->position.line, field3->key->position.column);
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


    if (field1->value != NULL)
    {
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_resolve(program, NULL, field1->value, response1, SELECT_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        else
        if (r1 == 0)
        {
            semantic_error(program, field1->value, "reference not found");
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
                    if ((item1->flag & NODE_FLAG_NEW) == NODE_FLAG_NEW)
                    {
                        semantic_error(program, field1->value, "type expected");
                        return -1;
                    }
                }
                else
                if (item1->kind == NODE_KIND_GENERIC)
                {
                }
                else
                {
                    semantic_error(program, field1->value, "type expected");
                    return -1;
                }
            }
        }
        
        list_destroy(response1);
    }
    else
    {
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_resolve(program, NULL, field1->key, response1, SELECT_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        else
        if (r1 == 0)
        {
            semantic_error(program, field1->key, "reference not found");
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
                    if ((item1->flag & NODE_FLAG_NEW) == NODE_FLAG_NEW)
                    {
                        semantic_error(program, field1->key, "type expected");
                        return -1;
                    }
                }
                else
                if (item1->kind == NODE_KIND_GENERIC)
                {
                }
                else
                {
                    semantic_error(program, field1->key, "type expected");
                    return -1;
                }
            }
        }
        
        list_destroy(response1);
    }

    return 1;
}

static int32_t
semantic_fields(program_t *program, node_t *node)
{
	node_block_t *fields = (node_block_t *)node->value;
    ilist_t *a1;
    for (a1 = fields->list->begin;a1 != fields->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        int32_t result;
        result = semantic_field(program, item1);
        if (result == -1)
        {
            return -1;
        }
    }
	return 1;
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
                    if (item2->kind == NODE_KIND_TRY)
                    {
                        node_try_t *try2 = (node_try_t *)item2->value;
                        if (semantic_idcmp(for1->key, try2->key) == 1)
                        {
                            semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                try2->key->position.line, try2->key->position.column);
                            return -1;
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
                    if (item2->kind == NODE_KIND_TRY)
                    {
                        node_try_t *try2 = (node_try_t *)item2->value;
                        if (semantic_idcmp(for1->key, try2->key) == 1)
                        {
                            semantic_error(program, for1->key, "already defined, previous in (%lld:%lld)",
                                try2->key->position.line, try2->key->position.column);
                            return -1;
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

    if (parameter1->type != NULL)
    {
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_resolve(program, NULL, parameter1->type, response1, SELECT_FLAG_NONE);
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
            ilist_t *a1;
            for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_CLASS)
                {
                    if ((item1->flag & NODE_FLAG_NEW) == NODE_FLAG_NEW)
                    {
                        semantic_error(program, parameter1->type, "type expected");
                        return -1;
                    }
                }
                else
                if (item1->kind == NODE_KIND_FN)
                {
                    node_fn_t *fn1 = (node_fn_t *)item1->value;
                    if (fn1->generics != NULL)
                    {
                        int32_t r2 = semantic_generics(program, fn1->generics);
                        if (r2 == -1)
                        {
                            return -1;
                        }
                    }

                    if (fn1->parameters != NULL)
                    {
                        int32_t r2 = semantic_parameters(program, fn1->parameters);
                        if (r2 == -1)
                        {
                            return -1;
                        }
                    }

                    if (fn1->result != NULL)
                    {
                        list_t *response2 = list_create();
                        if (response2 == NULL)
                        {
                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }

                        int32_t r3 = semantic_resolve(program, NULL, fn1->result, response2, SELECT_FLAG_NONE);
                        if (r3 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r3 == 0)
                        {
                            semantic_error(program,fn1->result, "reference not found");
                            return -1;
                        }
                        else
                        if (r3 == 1)
                        {
                            ilist_t *a2;
                            for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
                            {
                                node_t *item2 = (node_t *)a2->value;
                                if (item2->kind == NODE_KIND_CLASS)
                                {
                                    if ((item2->flag & NODE_FLAG_NEW) == NODE_FLAG_NEW)
                                    {
                                        semantic_error(program, fn1->result, "type expected");
                                        return -1;
                                    }
                                }
                                else
                                if (item2->kind == NODE_KIND_FN)
                                {
                                    node_fn_t *fn1 = (node_fn_t *)item2->value;
                                    if (fn1->generics != NULL)
                                    {
                                        int32_t r2 = semantic_generics(program, fn1->generics);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                    }

                                    if (fn1->parameters != NULL)
                                    {
                                        int32_t r2 = semantic_parameters(program, fn1->parameters);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                    }

                                    if (fn1->result != NULL)
                                    {
                                        
                                    }
                                }
                                else
                                if (item2->kind == NODE_KIND_ENUM)
                                {
                                }
                                else
                                {
                                    semantic_error(program, fn1->result, "type expected");
                                    return -1;
                                }
                            }
                        }

                        list_destroy(response2);
                    }
                }
                else
                if (item1->kind == NODE_KIND_ENUM)
                {
                }
                else
                if (item1->kind == NODE_KIND_GENERIC)
                {
                }
                else
                {
                    semantic_error(program, parameter1->type, "type expected");
                    return -1;
                }
            }
        }

        list_destroy(response1);
    }

    if (parameter1->value != NULL)
    {
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_expression(program, NULL, parameter1->value, response1, SELECT_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        if (r1 == 0)
        {
            semantic_error(program, parameter1->value, "reference not found");
            return -1;
        }
        else
        if (r1 == 1)
        {
            ilist_t *a1;
            for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
            {
                node_t *item2 = (node_t *)a1->value;
                if (item2->kind == NODE_KIND_VAR)
                {
                    node_var_t *var2 = (node_var_t *)item2->value;
                    if (var2->value_update != NULL)
                    {
                        parameter1->value_update = var2->value_update;
                    }
                    else
                    {
                        semantic_error(program, item2, "unitialized, encounter in (%lld:%lld)",
                            parameter1->value->position.line, parameter1->value->position.column);
                        return -1;
                    }
                }
                else
                if (item2->kind == NODE_KIND_ENTITY)
                {
                    node_entity_t *entity2 = (node_entity_t *)item2->value;
                    if (entity2->value_update != NULL)
                    {
                        parameter1->value_update = entity2->value_update;
                    }
                    else
                    {
                        semantic_error(program, item2, "unitialized, encounter in (%lld:%lld)",
                            parameter1->value->position.line, parameter1->value->position.column);
                        return -1;
                    }
                }
                else
                if (item2->kind == NODE_KIND_PROPERTY)
                {
                    node_property_t *property2 = (node_property_t *)item2->value;
                    if (property2->value_update != NULL)
                    {
                        parameter1->value_update = property2->value_update;
                    }
                    else
                    {
                        semantic_error(program, item2, "unitialized, encounter in (%lld:%lld)",
                            parameter1->value->position.line, parameter1->value->position.column);
                        return -1;
                    }
                }
                else
                if (item2->kind == NODE_KIND_PARAMETER)
                {
                    node_parameter_t *parameter2 = (node_parameter_t *)item2->value;
                    if (parameter2->value_update != NULL)
                    {
                        parameter1->value_update = parameter2->value_update;
                    }
                    else
                    {
                        semantic_error(program, item2, "unitialized, encounter in (%lld:%lld)",
                            parameter1->value->position.line, parameter1->value->position.column);
                        return -1;
                    }
                }
                else
                if (item2->kind == NODE_KIND_MEMBER)
                {
                    node_member_t *member2 = (node_member_t *)item2->value;
                    if (member2->value_update != NULL)
                    {
                        parameter1->value_update = member2->value_update;
                    }
                    else
                    {
                        semantic_error(program, item2, "unitialized, encounter in (%lld:%lld)",
                            parameter1->value->position.line, parameter1->value->position.column);
                        return -1;
                    }
                }
                else
                {
                    parameter1->value_update = item2;
                }
            }
        }

        list_destroy(response1);
    }

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

        int32_t r1 = semantic_resolve(program, NULL, generic1->type, response1, SELECT_FLAG_NONE);
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
            ilist_t *a1;
            for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_CLASS)
                {
                    if ((item1->flag & NODE_FLAG_NEW) == NODE_FLAG_NEW)
                    {
                        semantic_error(program, generic1->type, "type expected");
                        return -1;
                    }
                }
                else
                if (item1->kind == NODE_KIND_GENERIC)
                {
                }
                else
                {
                    semantic_error(program, generic1->type, "type expected");
                    return -1;
                }
            }
        }

        list_destroy(response1);
    }

    if (generic1->value != NULL)
    {
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_resolve(program, NULL, generic1->value, response1, SELECT_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        else
        if (r1 == 0)
        {
            semantic_error(program, generic1->value, "reference not found");
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
                    if ((item1->flag & NODE_FLAG_NEW) == NODE_FLAG_NEW)
                    {
                        semantic_error(program, generic1->value, "type expected");
                        return -1;
                    }
                }
                else
                if (item1->kind == NODE_KIND_GENERIC)
                {
                }
                else
                {
                    semantic_error(program, generic1->value, "type expected");
                    return -1;
                }
            }
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

    if (catch1->parameters != NULL)
    {
        int32_t r1 = semantic_parameters(program, catch1->parameters);
        if (r1 == -1)
        {
            return -1;
        }
    }

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
                            if (item3->kind == NODE_KIND_ENTITY)
                            {
                                node_entity_t *entity1 = (node_entity_t *)item3->value;
                                if (semantic_idcmp(try1->key, entity1->key) == 1)
                                {
                                    semantic_error(program, try1->key, "already defined, previous in (%lld:%lld)",
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
                            if (item3->kind == NODE_KIND_ENTITY)
                            {
                                node_entity_t *entity1 = (node_entity_t *)item3->value;
                                if (semantic_idcmp(try1->key, entity1->key) == 1)
                                {
                                    semantic_error(program, try1->key, "already defined, previous in (%lld:%lld)",
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
                        semantic_error(program, try1->key, "already defined, previous in (%lld:%lld)",
                            try2->key->position.line, try2->key->position.column);
                        return -1;
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
                            if (item3->kind == NODE_KIND_ENTITY)
                            {
                                node_entity_t *entity1 = (node_entity_t *)item3->value;
                                if (semantic_idcmp(try1->key, entity1->key) == 1)
                                {
                                    semantic_error(program, try1->key, "already defined, previous in (%lld:%lld)",
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
                        if (semantic_idcmp(try1->key, generic3->key) == 1)
                        {
                            semantic_error(program, try1->key, "already defined, previous in (%lld:%lld)",
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
                            if (item4->kind == NODE_KIND_ENTITY)
                            {
                                node_entity_t *entity4 = (node_entity_t *)item4->value;
                                if (semantic_idcmp(entity4->key, generic2->key) == 1)
                                {
                                    semantic_error(program, entity4->key, "already defined, previous in (%lld:%lld)",
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

    if (var1->type != NULL)
    {
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_resolve(program, NULL, var1->type, response1, SELECT_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        else
        if (r1 == 0)
        {
            semantic_error(program, var1->type, "reference not found");
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
                    if ((item1->flag & NODE_FLAG_NEW) == NODE_FLAG_NEW)
                    {
                        semantic_error(program, var1->type, "type expected");
                        return -1;
                    }
                }
                else
                if (item1->kind == NODE_KIND_FN)
                {
                    node_fn_t *fn1 = (node_fn_t *)item1->value;
                    if (fn1->generics != NULL)
                    {
                        int32_t r2 = semantic_generics(program, fn1->generics);
                        if (r2 == -1)
                        {
                            return -1;
                        }
                    }

                    if (fn1->parameters != NULL)
                    {
                        int32_t r2 = semantic_parameters(program, fn1->parameters);
                        if (r2 == -1)
                        {
                            return -1;
                        }
                    }

                    if (fn1->result != NULL)
                    {
                        list_t *response2 = list_create();
                        if (response2 == NULL)
                        {
                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }

                        int32_t r3 = semantic_resolve(program, NULL, fn1->result, response2, SELECT_FLAG_NONE);
                        if (r3 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r3 == 0)
                        {
                            semantic_error(program,fn1->result, "reference not found");
                            return -1;
                        }
                        else
                        if (r3 == 1)
                        {
                            ilist_t *a2;
                            for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
                            {
                                node_t *item2 = (node_t *)a2->value;
                                if (item2->kind == NODE_KIND_CLASS)
                                {
                                    if ((item2->flag & NODE_FLAG_NEW) == NODE_FLAG_NEW)
                                    {
                                        semantic_error(program, fn1->result, "type expected");
                                        return -1;
                                    }
                                }
                                else
                                if (item2->kind == NODE_KIND_FN)
                                {
                                    node_fn_t *fn1 = (node_fn_t *)item2->value;
                                    if (fn1->generics != NULL)
                                    {
                                        int32_t r2 = semantic_generics(program, fn1->generics);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                    }

                                    if (fn1->parameters != NULL)
                                    {
                                        int32_t r2 = semantic_parameters(program, fn1->parameters);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                    }

                                    if (fn1->result != NULL)
                                    {
                                        
                                    }
                                }
                                else
                                if (item2->kind == NODE_KIND_ENUM)
                                {
                                }
                                else
                                {
                                    semantic_error(program, fn1->result, "type expected");
                                    return -1;
                                }
                            }
                        }

                        list_destroy(response2);
                    }
                }
                else
                if (item1->kind == NODE_KIND_ENUM)
                {
                }
                else
                if (item1->kind == NODE_KIND_GENERIC)
                {
                }
                else
                {
                    semantic_error(program, var1->type, "type expected");
                    return -1;
                }
            }
        }

        list_destroy(response1);
    }

    if (var1->value != NULL)
    {
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_expression(program, NULL, var1->value, response1, SELECT_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        if (r1 == 0)
        {
            semantic_error(program, var1->value, "reference not found");
            return -1;
        }
        else
        if (r1 == 1)
        {
            ilist_t *a1;
            for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
            {
                node_t *item2 = (node_t *)a1->value;
                if (item2->kind == NODE_KIND_VAR)
                {
                    node_var_t *var2 = (node_var_t *)item2->value;
                    if (var2->value_update != NULL)
                    {
                        if (var1->key->kind == NODE_KIND_SET)
                        {
                            node_t *node1 = var1->key;
                            node_block_t *block1 = (node_block_t *)node1->value;

                            ilist_t *a2;
                            for (a2 = block1->list->begin;a2 != block1->list->end;a2 = a2->next)
                            {
                                node_t *item3 = (node_t *)a2->value;
                                if (item3->kind == NODE_KIND_ENTITY)
                                {
                                    node_entity_t *entity1 = (node_entity_t *)item3->value;

                                    node_t *node2 = var2->value_update;
                                    if (node2->kind == NODE_KIND_CLASS)
                                    {
                                        node_class_t *class1 = (node_class_t *)node2->value;

                                        node_t *node3 = class1->block;
                                        node_block_t *block2 = (node_block_t *)node3->value;

                                        ilist_t *a3;
                                        for (a3 = block2->list->begin;a3 != block2->list->end;a3 = a3->next)
                                        {
                                            node_t *item4 = (node_t *)a3->value;
                                            if (item4->kind == NODE_KIND_CLASS)
                                            {
                                                node_class_t *class2 = (node_class_t *)item4->value;

                                                if (entity1->value != NULL)
                                                {
                                                    if (semantic_idcmp(class2->key, entity1->value) == 1)
                                                    {
                                                        entity1->value_update = item4;
                                                    }
                                                }
                                                else
                                                {
                                                    if (semantic_idcmp(class2->key, entity1->key) == 1)
                                                    {
                                                        entity1->value_update = item4;
                                                    }
                                                }
                                            }
                                            else
                                            if (item4->kind == NODE_KIND_FUN)
                                            {
                                                node_fun_t *fun2 = (node_fun_t *)item4->value;

                                                if (entity1->value != NULL)
                                                {
                                                    if (semantic_idcmp(fun2->key, entity1->value) == 1)
                                                    {
                                                        entity1->value_update = item4;
                                                    }
                                                }
                                                else
                                                {
                                                    if (semantic_idcmp(fun2->key, entity1->key) == 1)
                                                    {
                                                        entity1->value_update = item4;
                                                    }
                                                }
                                            }
                                            else
                                            if (item4->kind == NODE_KIND_ENUM)
                                            {
                                                node_enum_t *enum2 = (node_enum_t *)item4->value;

                                                if (entity1->value != NULL)
                                                {
                                                    if (semantic_idcmp(enum2->key, entity1->value) == 1)
                                                    {
                                                        entity1->value_update = item4;
                                                    }
                                                }
                                                else
                                                {
                                                    if (semantic_idcmp(enum2->key, entity1->key) == 1)
                                                    {
                                                        entity1->value_update = item4;
                                                    }
                                                }
                                            }
                                            else
                                            if (item4->kind == NODE_KIND_PROPERTY)
                                            {
                                                node_property_t *property2 = (node_property_t *)item4->value;

                                                if (entity1->value != NULL)
                                                {
                                                    if (semantic_idcmp(property2->key, entity1->value) == 1)
                                                    {
                                                        entity1->value_update = item4;
                                                    }
                                                }
                                                else
                                                {
                                                    if (semantic_idcmp(property2->key, entity1->key) == 1)
                                                    {
                                                        entity1->value_update = item4;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    else
                                    {
                                        semantic_error(program, node2, "not selectable, encounter in (%lld:%lld)",
                                            var1->key->position.line, var1->key->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                        else
                        {
                            var1->value_update = var2->value_update;
                        }
                    }
                    else
                    {
                        semantic_error(program, item2, "unitialized, encounter in (%lld:%lld)",
                            var1->value->position.line, var1->value->position.column);
                        return -1;
                    }
                }
                else
                if (item2->kind == NODE_KIND_ENTITY)
                {
                    node_entity_t *entity2 = (node_entity_t *)item2->value;
                    if (entity2->value_update != NULL)
                    {
                        if (var1->key->kind == NODE_KIND_SET)
                        {
                            node_t *node1 = var1->key;
                            node_block_t *block1 = (node_block_t *)node1->value;

                            ilist_t *a2;
                            for (a2 = block1->list->begin;a2 != block1->list->end;a2 = a2->next)
                            {
                                node_t *item3 = (node_t *)a2->value;
                                if (item3->kind == NODE_KIND_ENTITY)
                                {
                                    node_entity_t *entity1 = (node_entity_t *)item3->value;

                                    node_t *node2 = entity2->value_update;
                                    if (node2->kind == NODE_KIND_CLASS)
                                    {
                                        node_class_t *class1 = (node_class_t *)node2->value;

                                        node_t *node3 = class1->block;
                                        node_block_t *block2 = (node_block_t *)node3->value;

                                        ilist_t *a3;
                                        for (a3 = block2->list->begin;a3 != block2->list->end;a3 = a3->next)
                                        {
                                            node_t *item4 = (node_t *)a3->value;
                                            if (item4->kind == NODE_KIND_CLASS)
                                            {
                                                node_class_t *class2 = (node_class_t *)item4->value;

                                                if (entity1->value != NULL)
                                                {
                                                    if (semantic_idcmp(class2->key, entity1->value) == 1)
                                                    {
                                                        entity1->value_update = item4;
                                                    }
                                                }
                                                else
                                                {
                                                    if (semantic_idcmp(class2->key, entity1->key) == 1)
                                                    {
                                                        entity1->value_update = item4;
                                                    }
                                                }
                                            }
                                            else
                                            if (item4->kind == NODE_KIND_FUN)
                                            {
                                                node_fun_t *fun2 = (node_fun_t *)item4->value;

                                                if (entity1->value != NULL)
                                                {
                                                    if (semantic_idcmp(fun2->key, entity1->value) == 1)
                                                    {
                                                        entity1->value_update = item4;
                                                    }
                                                }
                                                else
                                                {
                                                    if (semantic_idcmp(fun2->key, entity1->key) == 1)
                                                    {
                                                        entity1->value_update = item4;
                                                    }
                                                }
                                            }
                                            else
                                            if (item4->kind == NODE_KIND_ENUM)
                                            {
                                                node_enum_t *enum2 = (node_enum_t *)item4->value;

                                                if (entity1->value != NULL)
                                                {
                                                    if (semantic_idcmp(enum2->key, entity1->value) == 1)
                                                    {
                                                        entity1->value_update = item4;
                                                    }
                                                }
                                                else
                                                {
                                                    if (semantic_idcmp(enum2->key, entity1->key) == 1)
                                                    {
                                                        entity1->value_update = item4;
                                                    }
                                                }
                                            }
                                            else
                                            if (item4->kind == NODE_KIND_PROPERTY)
                                            {
                                                node_property_t *property2 = (node_property_t *)item4->value;

                                                if (entity1->value != NULL)
                                                {
                                                    if (semantic_idcmp(property2->key, entity1->value) == 1)
                                                    {
                                                        entity1->value_update = item4;
                                                    }
                                                }
                                                else
                                                {
                                                    if (semantic_idcmp(property2->key, entity1->key) == 1)
                                                    {
                                                        entity1->value_update = item4;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    else
                                    {
                                        semantic_error(program, node2, "not selectable, encounter in (%lld:%lld)",
                                            var1->key->position.line, var1->key->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                        else
                        {
                            var1->value_update = entity2->value_update;
                        }
                    }
                    else
                    {
                        semantic_error(program, item2, "unitialized, encounter in (%lld:%lld)",
                            var1->value->position.line, var1->value->position.column);
                        return -1;
                    }
                }
                else
                if (item2->kind == NODE_KIND_PROPERTY)
                {
                    node_property_t *property2 = (node_property_t *)item2->value;
                    if (property2->value_update != NULL)
                    {
                        if (var1->key->kind == NODE_KIND_SET)
                        {
                            node_t *node1 = var1->key;
                            node_block_t *block1 = (node_block_t *)node1->value;

                            ilist_t *a2;
                            for (a2 = block1->list->begin;a2 != block1->list->end;a2 = a2->next)
                            {
                                node_t *item3 = (node_t *)a2->value;
                                if (item3->kind == NODE_KIND_ENTITY)
                                {
                                    node_entity_t *entity1 = (node_entity_t *)item3->value;

                                    node_t *node2 = property2->value_update;
                                    if (node2->kind == NODE_KIND_CLASS)
                                    {
                                        node_class_t *class1 = (node_class_t *)node2->value;

                                        node_t *node3 = class1->block;
                                        node_block_t *block2 = (node_block_t *)node3->value;

                                        ilist_t *a3;
                                        for (a3 = block2->list->begin;a3 != block2->list->end;a3 = a3->next)
                                        {
                                            node_t *item4 = (node_t *)a3->value;
                                            if (item4->kind == NODE_KIND_CLASS)
                                            {
                                                node_class_t *class2 = (node_class_t *)item4->value;

                                                if (entity1->value != NULL)
                                                {
                                                    if (semantic_idcmp(class2->key, entity1->value) == 1)
                                                    {
                                                        entity1->value_update = item4;
                                                    }
                                                }
                                                else
                                                {
                                                    if (semantic_idcmp(class2->key, entity1->key) == 1)
                                                    {
                                                        entity1->value_update = item4;
                                                    }
                                                }
                                            }
                                            else
                                            if (item4->kind == NODE_KIND_FUN)
                                            {
                                                node_fun_t *fun2 = (node_fun_t *)item4->value;

                                                if (entity1->value != NULL)
                                                {
                                                    if (semantic_idcmp(fun2->key, entity1->value) == 1)
                                                    {
                                                        entity1->value_update = item4;
                                                    }
                                                }
                                                else
                                                {
                                                    if (semantic_idcmp(fun2->key, entity1->key) == 1)
                                                    {
                                                        entity1->value_update = item4;
                                                    }
                                                }
                                            }
                                            else
                                            if (item4->kind == NODE_KIND_ENUM)
                                            {
                                                node_enum_t *enum2 = (node_enum_t *)item4->value;

                                                if (entity1->value != NULL)
                                                {
                                                    if (semantic_idcmp(enum2->key, entity1->value) == 1)
                                                    {
                                                        entity1->value_update = item4;
                                                    }
                                                }
                                                else
                                                {
                                                    if (semantic_idcmp(enum2->key, entity1->key) == 1)
                                                    {
                                                        entity1->value_update = item4;
                                                    }
                                                }
                                            }
                                            else
                                            if (item4->kind == NODE_KIND_PROPERTY)
                                            {
                                                node_property_t *property2 = (node_property_t *)item4->value;

                                                if (entity1->value != NULL)
                                                {
                                                    if (semantic_idcmp(property2->key, entity1->value) == 1)
                                                    {
                                                        entity1->value_update = item4;
                                                    }
                                                }
                                                else
                                                {
                                                    if (semantic_idcmp(property2->key, entity1->key) == 1)
                                                    {
                                                        entity1->value_update = item4;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    else
                                    {
                                        semantic_error(program, node2, "not selectable, encounter in (%lld:%lld)",
                                            var1->key->position.line, var1->key->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                        else
                        {
                            var1->value_update = property2->value_update;
                        }
                    }
                    else
                    {
                        semantic_error(program, item2, "unitialized, encounter in (%lld:%lld)",
                            var1->value->position.line, var1->value->position.column);
                        return -1;
                    }
                }
                else
                if (item2->kind == NODE_KIND_PARAMETER)
                {
                    node_parameter_t *parameter2 = (node_parameter_t *)item2->value;
                    if (parameter2->value_update != NULL)
                    {
                        if (var1->key->kind == NODE_KIND_SET)
                        {
                            node_t *node1 = var1->key;
                            node_block_t *block1 = (node_block_t *)node1->value;

                            ilist_t *a2;
                            for (a2 = block1->list->begin;a2 != block1->list->end;a2 = a2->next)
                            {
                                node_t *item3 = (node_t *)a2->value;
                                if (item3->kind == NODE_KIND_ENTITY)
                                {
                                    node_entity_t *entity1 = (node_entity_t *)item3->value;

                                    node_t *node2 = parameter2->value_update;
                                    if (node2->kind == NODE_KIND_CLASS)
                                    {
                                        node_class_t *class1 = (node_class_t *)node2->value;

                                        node_t *node3 = class1->block;
                                        node_block_t *block2 = (node_block_t *)node3->value;

                                        ilist_t *a3;
                                        for (a3 = block2->list->begin;a3 != block2->list->end;a3 = a3->next)
                                        {
                                            node_t *item4 = (node_t *)a3->value;
                                            if (item4->kind == NODE_KIND_CLASS)
                                            {
                                                node_class_t *class2 = (node_class_t *)item4->value;

                                                if (entity1->value != NULL)
                                                {
                                                    if (semantic_idcmp(class2->key, entity1->value) == 1)
                                                    {
                                                        entity1->value_update = item4;
                                                    }
                                                }
                                                else
                                                {
                                                    if (semantic_idcmp(class2->key, entity1->key) == 1)
                                                    {
                                                        entity1->value_update = item4;
                                                    }
                                                }
                                            }
                                            else
                                            if (item4->kind == NODE_KIND_FUN)
                                            {
                                                node_fun_t *fun2 = (node_fun_t *)item4->value;

                                                if (entity1->value != NULL)
                                                {
                                                    if (semantic_idcmp(fun2->key, entity1->value) == 1)
                                                    {
                                                        entity1->value_update = item4;
                                                    }
                                                }
                                                else
                                                {
                                                    if (semantic_idcmp(fun2->key, entity1->key) == 1)
                                                    {
                                                        entity1->value_update = item4;
                                                    }
                                                }
                                            }
                                            else
                                            if (item4->kind == NODE_KIND_ENUM)
                                            {
                                                node_enum_t *enum2 = (node_enum_t *)item4->value;

                                                if (entity1->value != NULL)
                                                {
                                                    if (semantic_idcmp(enum2->key, entity1->value) == 1)
                                                    {
                                                        entity1->value_update = item4;
                                                    }
                                                }
                                                else
                                                {
                                                    if (semantic_idcmp(enum2->key, entity1->key) == 1)
                                                    {
                                                        entity1->value_update = item4;
                                                    }
                                                }
                                            }
                                            else
                                            if (item4->kind == NODE_KIND_PROPERTY)
                                            {
                                                node_property_t *property2 = (node_property_t *)item4->value;

                                                if (entity1->value != NULL)
                                                {
                                                    if (semantic_idcmp(property2->key, entity1->value) == 1)
                                                    {
                                                        entity1->value_update = item4;
                                                    }
                                                }
                                                else
                                                {
                                                    if (semantic_idcmp(property2->key, entity1->key) == 1)
                                                    {
                                                        entity1->value_update = item4;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    else
                                    {
                                        semantic_error(program, node2, "not selectable, encounter in (%lld:%lld)",
                                            var1->key->position.line, var1->key->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                        else
                        {
                            var1->value_update = parameter2->value_update;
                        }
                    }
                    else
                    {
                        semantic_error(program, item2, "unitialized, encounter in (%lld:%lld)",
                            var1->value->position.line, var1->value->position.column);
                        return -1;
                    }
                }
                else
                if (item2->kind == NODE_KIND_MEMBER)
                {
                    node_member_t *member2 = (node_member_t *)item2->value;
                    if (member2->value_update != NULL)
                    {
                        if (var1->key->kind == NODE_KIND_SET)
                        {
                            node_t *node1 = var1->key;
                            node_block_t *block1 = (node_block_t *)node1->value;

                            ilist_t *a2;
                            for (a2 = block1->list->begin;a2 != block1->list->end;a2 = a2->next)
                            {
                                node_t *item3 = (node_t *)a2->value;
                                if (item3->kind == NODE_KIND_ENTITY)
                                {
                                    node_entity_t *entity1 = (node_entity_t *)item3->value;

                                    node_t *node2 = member2->value_update;
                                    if (node2->kind == NODE_KIND_CLASS)
                                    {
                                        node_class_t *class1 = (node_class_t *)node2->value;

                                        node_t *node3 = class1->block;
                                        node_block_t *block2 = (node_block_t *)node3->value;

                                        ilist_t *a3;
                                        for (a3 = block2->list->begin;a3 != block2->list->end;a3 = a3->next)
                                        {
                                            node_t *item4 = (node_t *)a3->value;
                                            if (item4->kind == NODE_KIND_CLASS)
                                            {
                                                node_class_t *class2 = (node_class_t *)item4->value;

                                                if (entity1->value != NULL)
                                                {
                                                    if (semantic_idcmp(class2->key, entity1->value) == 1)
                                                    {
                                                        entity1->value_update = item4;
                                                    }
                                                }
                                                else
                                                {
                                                    if (semantic_idcmp(class2->key, entity1->key) == 1)
                                                    {
                                                        entity1->value_update = item4;
                                                    }
                                                }
                                            }
                                            else
                                            if (item4->kind == NODE_KIND_FUN)
                                            {
                                                node_fun_t *fun2 = (node_fun_t *)item4->value;

                                                if (entity1->value != NULL)
                                                {
                                                    if (semantic_idcmp(fun2->key, entity1->value) == 1)
                                                    {
                                                        entity1->value_update = item4;
                                                    }
                                                }
                                                else
                                                {
                                                    if (semantic_idcmp(fun2->key, entity1->key) == 1)
                                                    {
                                                        entity1->value_update = item4;
                                                    }
                                                }
                                            }
                                            else
                                            if (item4->kind == NODE_KIND_ENUM)
                                            {
                                                node_enum_t *enum2 = (node_enum_t *)item4->value;

                                                if (entity1->value != NULL)
                                                {
                                                    if (semantic_idcmp(enum2->key, entity1->value) == 1)
                                                    {
                                                        entity1->value_update = item4;
                                                    }
                                                }
                                                else
                                                {
                                                    if (semantic_idcmp(enum2->key, entity1->key) == 1)
                                                    {
                                                        entity1->value_update = item4;
                                                    }
                                                }
                                            }
                                            else
                                            if (item4->kind == NODE_KIND_PROPERTY)
                                            {
                                                node_property_t *property2 = (node_property_t *)item4->value;

                                                if (entity1->value != NULL)
                                                {
                                                    if (semantic_idcmp(property2->key, entity1->value) == 1)
                                                    {
                                                        entity1->value_update = item4;
                                                    }
                                                }
                                                else
                                                {
                                                    if (semantic_idcmp(property2->key, entity1->key) == 1)
                                                    {
                                                        entity1->value_update = item4;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    else
                                    {
                                        semantic_error(program, node2, "not selectable, encounter in (%lld:%lld)",
                                            var1->key->position.line, var1->key->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                        else
                        {
                            var1->value_update = member2->value_update;
                        }
                    }
                    else
                    {
                        semantic_error(program, item2, "unitialized, encounter in (%lld:%lld)",
                            var1->value->position.line, var1->value->position.column);
                        return -1;
                    }
                }
                else
                {
                    var1->value_update = item2;
                }
            }
        }

        list_destroy(response1);
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
    else
    {
        return semantic_assign(program, node);
    }
    return 0;
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
	node_fun_t *fun1 = (node_fun_t *)node->value;

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
                           semantic_error(program, fun1->key, "already defined, previous in (%lld:%lld)",
                                generic1->key->position.line, generic1->key->position.column);
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
                            semantic_error(program, fun1->key, "already defined, previous in (%lld:%lld)",
                                heritage1->key->position.line, heritage1->key->position.column);
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

                if (item2->id == sub->id)
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
                            node_t *node3 = class3->block;
                            node_block_t *block3 = (node_block_t *)node3->value;

                            ilist_t *a3;
                            for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                            {
                                node_t *item3 = (node_t *)a3->value;
                                if (item3->kind == NODE_KIND_FUN)
                                {
                                    node_fun_t *fun3 = (node_fun_t *)item3->value;
                                    if (semantic_idstrcmp(fun3->key, "Constructor") == 1)
                                    {
                                        node_t *nps1 = fun3->parameters;
                                        node_t *nps2 = fun1->parameters;
                                        int32_t r2 = semantic_eqaul_psps(program, nps1, nps2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            semantic_error(program, fun1->key, "already defined, previous in (%lld:%lld)",
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

                    if (semantic_idcmp(fun1->key, enum2->key) == 1)
                    {
                        semantic_error(program, fun1->key, "already defined, previous in (%lld:%lld)",
                            enum2->key->position.line, enum2->key->position.column);
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
                                semantic_error(program, fun1->key, "already defined, previous in (%lld:%lld)",
                                    fun2->key->position.line, fun2->key->position.column);
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
                        semantic_error(program, fun1->key, "already defined, previous in (%lld:%lld)",
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

    if (fun1->generics != NULL)
    {
        int32_t r1 = semantic_generics(program, fun1->generics);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (fun1->parameters != NULL)
    {
        int32_t r1 = semantic_parameters(program, fun1->parameters);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (fun1->body != NULL)
    {
        int32_t r1 = semantic_body(program, fun1->body);
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
                                semantic_error(program, enum1->key, "already defined, previous in (%lld:%lld)",
                                    package3->key->position.line, package3->key->position.column);
                                return -1;
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
                        semantic_error(program, enum1->key, "already defined, previous in (%lld:%lld)",
                            class3->key->position.line, class3->key->position.column);
                        return -1;
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
                        semantic_error(program, enum1->key, "already defined, previous in (%lld:%lld)",
                            class3->key->position.line, class3->key->position.column);
                        return -1;
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
                    node_fun_t *fun2 = (node_fun_t *)item2->value;

                    if (semantic_idcmp(enum1->key, fun2->key) == 1)
                    {
                        semantic_error(program, enum1->key, "already defined, previous in (%lld:%lld)",
                            fun2->key->position.line, fun2->key->position.column);
                        return -1;
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
                        semantic_error(program, package1->key, "already defined, previous in (%lld:%lld)",
                            package3->key->position.line, package3->key->position.column);
                        return -1;
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
                                semantic_error(program, package1->key, "already defined, previous in (%lld:%lld)",
                                    package3->key->position.line, package3->key->position.column);
                                return -1;
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
                        semantic_error(program, package1->key, "already defined, previous in (%lld:%lld)",
                            class3->key->position.line, class3->key->position.column);
                        return -1;
                    }
                }
                else
                if (item2->kind == NODE_KIND_ENUM)
                {
                    node_enum_t *enum2 = (node_enum_t *)item2->value;

                    if (semantic_idcmp(package1->key, enum2->key) == 1)
                    {
                        semantic_error(program, package1->key, "already defined, previous in (%lld:%lld)",
                            enum2->key->position.line, enum2->key->position.column);
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

    if (package1->generics != NULL)
    {
        int32_t r1 = semantic_generics(program, package1->generics);
        if (r1 == -1)
        {
            return -1;
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
                        semantic_error(program, property1->key, "already defined, previous in (%lld:%lld)",
                            class3->key->position.line, class3->key->position.column);
                        return -1;
                    }
                }
                else
                if (item2->kind == NODE_KIND_FUN)
                {
                    node_fun_t *fun2 = (node_fun_t *)item2->value;

                    if (semantic_idcmp(property1->key, fun2->key) == 1)
                    {
                        semantic_error(program, property1->key, "already defined, previous in (%lld:%lld)",
                            fun2->key->position.line, fun2->key->position.column);
                        return -1;
                    }
                }
                else
                if (item2->kind == NODE_KIND_ENUM)
                {
                    node_enum_t *enum2 = (node_enum_t *)item2->value;

                    if (semantic_idcmp(property1->key, enum2->key) == 1)
                    {
                        semantic_error(program, property1->key, "already defined, previous in (%lld:%lld)",
                            enum2->key->position.line, enum2->key->position.column);
                        return -1;
                    }
                }
                else
                if (item2->kind == NODE_KIND_PROPERTY)
                {
                    node_property_t *property2 = (node_property_t *)item2->value;

                    if (semantic_idcmp(property1->key, property2->key) == 1)
                    {
                        semantic_error(program, property1->key, "already defined, previous in (%lld:%lld)",
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
    
    if (property1->type != NULL)
    {
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_resolve(program, NULL, property1->type, response1, SELECT_FLAG_NONE);
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
        else
        if (r1 == 1)
        {
            ilist_t *a1;
            for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_CLASS)
                {
                    if ((item1->flag & NODE_FLAG_NEW) == NODE_FLAG_NEW)
                    {
                        semantic_error(program, property1->type, "type expected");
                        return -1;
                    }
                }
                else
                if (item1->kind == NODE_KIND_FN)
                {
                    node_fn_t *fn1 = (node_fn_t *)item1->value;
                    if (fn1->generics != NULL)
                    {
                        int32_t r2 = semantic_generics(program, fn1->generics);
                        if (r2 == -1)
                        {
                            return -1;
                        }
                    }

                    if (fn1->parameters != NULL)
                    {
                        int32_t r2 = semantic_parameters(program, fn1->parameters);
                        if (r2 == -1)
                        {
                            return -1;
                        }
                    }

                    if (fn1->result != NULL)
                    {
                        list_t *response2 = list_create();
                        if (response2 == NULL)
                        {
                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }

                        int32_t r3 = semantic_resolve(program, NULL, fn1->result, response2, SELECT_FLAG_NONE);
                        if (r3 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r3 == 0)
                        {
                            semantic_error(program,fn1->result, "reference not found");
                            return -1;
                        }
                        else
                        if (r3 == 1)
                        {
                            ilist_t *a2;
                            for (a2 = response2->begin;a2 != response2->end;a2 = a2->next)
                            {
                                node_t *item2 = (node_t *)a2->value;
                                if (item2->kind == NODE_KIND_CLASS)
                                {
                                    if ((item2->flag & NODE_FLAG_NEW) == NODE_FLAG_NEW)
                                    {
                                        semantic_error(program, fn1->result, "type expected");
                                        return -1;
                                    }
                                }
                                else
                                if (item2->kind == NODE_KIND_FN)
                                {
                                    node_fn_t *fn1 = (node_fn_t *)item2->value;
                                    if (fn1->generics != NULL)
                                    {
                                        int32_t r2 = semantic_generics(program, fn1->generics);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                    }

                                    if (fn1->parameters != NULL)
                                    {
                                        int32_t r2 = semantic_parameters(program, fn1->parameters);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                    }

                                    if (fn1->result != NULL)
                                    {
                                        
                                    }
                                }
                                else
                                if (item2->kind == NODE_KIND_ENUM)
                                {
                                }
                                else
                                {
                                    semantic_error(program, fn1->result, "type expected");
                                    return -1;
                                }
                            }
                        }

                        list_destroy(response2);
                    }
                }
                else
                if (item1->kind == NODE_KIND_ENUM)
                {
                }
                else
                if (item1->kind == NODE_KIND_GENERIC)
                {
                }
                else
                {
                    semantic_error(program, property1->type, "type expected");
                    return -1;
                }
            }
        }

        list_destroy(response1);
    }

    if (property1->value != NULL)
    {
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_expression(program, NULL, property1->value, response1, SELECT_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        if (r1 == 0)
        {
            semantic_error(program, property1->value, "reference not found");
            return -1;
        }
        else
        if (r1 == 1)
        {
            ilist_t *a1;
            for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
            {
                node_t *item2 = (node_t *)a1->value;
                if (item2->kind == NODE_KIND_VAR)
                {
                    node_var_t *var2 = (node_var_t *)item2->value;
                    if (var2->value_update != NULL)
                    {
                        property1->value_update = var2->value_update;
                    }
                    else
                    {
                        semantic_error(program, item2, "unitialized, encounter in (%lld:%lld)",
                            property1->value->position.line, property1->value->position.column);
                        return -1;
                    }
                }
                else
                if (item2->kind == NODE_KIND_ENTITY)
                {
                    node_entity_t *entity2 = (node_entity_t *)item2->value;
                    if (entity2->value_update != NULL)
                    {
                        property1->value_update = entity2->value_update;
                    }
                    else
                    {
                        semantic_error(program, item2, "unitialized, encounter in (%lld:%lld)",
                            property1->value->position.line, property1->value->position.column);
                        return -1;
                    }
                }
                else
                if (item2->kind == NODE_KIND_PROPERTY)
                {
                    node_property_t *property2 = (node_property_t *)item2->value;
                    if (property2->value_update != NULL)
                    {
                        property1->value_update = property2->value_update;
                    }
                    else
                    {
                        semantic_error(program, item2, "unitialized, encounter in (%lld:%lld)",
                            property1->value->position.line, property1->value->position.column);
                        return -1;
                    }
                }
                else
                if (item2->kind == NODE_KIND_PARAMETER)
                {
                    node_parameter_t *parameter2 = (node_parameter_t *)item2->value;
                    if (parameter2->value_update != NULL)
                    {
                        property1->value_update = parameter2->value_update;
                    }
                    else
                    {
                        semantic_error(program, item2, "unitialized, encounter in (%lld:%lld)",
                            property1->value->position.line, property1->value->position.column);
                        return -1;
                    }
                }
                else
                if (item2->kind == NODE_KIND_MEMBER)
                {
                    node_member_t *member2 = (node_member_t *)item2->value;
                    if (member2->value_update != NULL)
                    {
                        property1->value_update = member2->value_update;
                    }
                    else
                    {
                        semantic_error(program, item2, "unitialized, encounter in (%lld:%lld)",
                            property1->value->position.line, property1->value->position.column);
                        return -1;
                    }
                }
                else
                {
                    property1->value_update = item2;
                }
            }
        }

        list_destroy(response1);
    }

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

    if (heritage1->type != NULL)
    {
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_resolve(program, NULL, heritage1->type, response1, SELECT_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        else
        if (r1 == 0)
        {
            semantic_error(program, heritage1->type, "reference not found");
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
                    if ((item1->flag & NODE_FLAG_NEW) == NODE_FLAG_NEW)
                    {
                        semantic_error(program, heritage1->type, "type expected");
                        return -1;
                    }
                }
                else
                if (item1->kind == NODE_KIND_GENERIC)
                {
                }
                else
                {
                    semantic_error(program, heritage1->type, "type expected");
                    return -1;
                }
            }
        }

        list_destroy(response1);
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
                                semantic_error(program, class1->key, "already defined, previous in (%lld:%lld)",
                                    package1->key->position.line, package1->key->position.column);
                                return -1;
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
                        semantic_error(program, class1->key, "already defined, previous in (%lld:%lld)",
                            class3->key->position.line, class3->key->position.column);
                        return -1;
                    }
                }
                else
                if (item2->kind == NODE_KIND_ENUM)
                {
                    node_enum_t *enum2 = (node_enum_t *)item2->value;

                    if (semantic_idcmp(class1->key, enum2->key) == 1)
                    {
                        semantic_error(program, class1->key, "already defined, previous in (%lld:%lld)",
                            enum2->key->position.line, enum2->key->position.column);
                        return -1;
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
                            semantic_error(program, class1->key, "already defined, previous in (%lld:%lld)",
                                generic1->key->position.line, generic1->key->position.column);
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
                            semantic_error(program, class1->key, "already defined, previous in (%lld:%lld)",
                                heritage1->key->position.line, heritage1->key->position.column);
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
                            node_t *node3 = class1->block;
                            node_block_t *block3 = (node_block_t *)node3->value;

                            ilist_t *a3;
                            for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                            {
                                node_t *item3 = (node_t *)a3->value;
                                if (item3->kind == NODE_KIND_FUN)
                                {
                                    node_fun_t *fun3 = (node_fun_t *)item3->value;
                                    if (semantic_idstrcmp(fun3->key, "Constructor") == 1)
                                    {
                                        node_t *node4 = class3->block;
                                        node_block_t *block4 = (node_block_t *)node4->value;

                                        ilist_t *a4;
                                        for (a4 = block4->list->begin;a4 != block4->list->end;a4 = a4->next)
                                        {
                                            node_t *item4 = (node_t *)a4->value;
                                            if (item4->kind == NODE_KIND_FUN)
                                            {
                                                node_fun_t *fun4 = (node_fun_t *)item4->value;
                                                if (semantic_idstrcmp(fun4->key, "Constructor") == 1)
                                                {
                                                    node_t *nps1 = fun3->parameters;
                                                    node_t *nps2 = fun4->parameters;
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
                            node_t *node3 = class1->block;
                            node_block_t *block3 = (node_block_t *)node3->value;

                            ilist_t *a3;
                            for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                            {
                                node_t *item3 = (node_t *)a3->value;
                                if (item3->kind == NODE_KIND_FUN)
                                {
                                    node_fun_t *fun3 = (node_fun_t *)item3->value;
                                    if (semantic_idstrcmp(fun3->key, "Constructor") == 1)
                                    {
                                        node_t *nps1 = fun3->parameters;
                                        node_t *nps2 = fun2->parameters;
                                        int32_t r2 = semantic_eqaul_psps(program, nps1, nps2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            semantic_error(program, class1->key, "already defined, previous in (%lld:%lld)",
                                                fun2->key->position.line, fun2->key->position.column);
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
                        semantic_error(program, class1->key, "already defined, previous in (%lld:%lld)",
                            enum2->key->position.line, enum2->key->position.column);
                        return -1;
                    }
                }
                else
                if (item2->kind == NODE_KIND_PROPERTY)
                {
                    node_property_t *property2 = (node_property_t *)item2->value;

                    if (semantic_idcmp(class1->key, property2->key) == 1)
                    {
                        semantic_error(program, class1->key, "already defined, previous in (%lld:%lld)",
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