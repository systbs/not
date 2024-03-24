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
#include "parser.h"
#include "error.h"
#include "syntax.h"

static error_t *
syntax_error(program_t *program, node_t *node, const char *format, ...)
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
syntax_idcmp(node_t *n1, node_t *n2)
{
	node_basic_t *nb1 = (node_basic_t *)n1->value;
	node_basic_t *nb2 = (node_basic_t *)n2->value;

	//printf("%s %s\n", nb1->value, nb2->value);

	return (strcmp(nb1->value, nb2->value) == 0);
}

static int32_t
syntax_idstrcmp(node_t *n1, char *name)
{
	node_basic_t *nb1 = (node_basic_t *)n1->value;
	return (strcmp(nb1->value, name) == 0);
}

static int32_t
syntax_objidcmp(node_t *n1, node_t *n2)
{
    if (n1->kind == NODE_KIND_OBJECT)
    {
        node_block_t *node_object = (node_block_t *)n1->value;
        ilist_t *a1;
        for (a1 = node_object->list->begin;a1 != node_object->list->end;a1 = a1->next)
        {
            node_t *item = (node_t *)a1->value;
            if (item->kind == NODE_KIND_PROPERTY)
            {
                node_property_t *node_property = (node_property_t *)item->value;
                if (node_property->type != NULL)
                {
                    int32_t r1 = syntax_objidcmp(node_property->type, n2);
                    if (r1 == 1)
                    {
                        return 1;
                    }
                }
                else
                {
                    int32_t r1 = syntax_objidcmp(node_property->key, n2);
                    if (r1 == 1)
                    {
                        return 1;
                    }
                }
            }
        }
        return 0;
    }
    else 
    if (n1->kind == NODE_KIND_ID)
    {
        if (n2->kind == NODE_KIND_OBJECT)
        {
            node_object_t *node_object = (node_object_t *)n2->value;
            ilist_t *a1;
            for (a1 = node_object->list->begin;a1 != node_object->list->end;a1 = a1->next)
            {
                node_t *item = (node_t *)a1->value;
                if (item->kind == NODE_KIND_PROPERTY)
                {
                    node_property_t *node_property = (node_property_t *)item->value;
                    if (node_property->type != NULL)
                    {
                        int32_t r1 = syntax_objidcmp(n1, node_property->type);
                        if (r1 == 1)
                        {
                            return 1;
                        }
                    }
                    else
                    {
                        int32_t r1 = syntax_objidcmp(n1, node_property->key);
                        if (r1 == 1)
                        {
                            return 1;
                        }
                    }
                }
            }
            return 0;
        }
        else 
        if (n2->kind == NODE_KIND_ID)
        {
            return (syntax_idcmp(n1, n2) == 1);
        }
    }
    return 0;
}


static int32_t
syntax_postfix(program_t *program, node_t *node, list_t *response, node_t *applicant);

static int32_t
syntax_expression(program_t *program, node_t *node, list_t *response);

static int32_t
syntax_body(program_t *program, node_t *node);


/*
 * @Work: n1 is subset of n2
 * @Return: 1 if is a subset otherwise 0 
*/
static int32_t
syntax_subset(program_t *program, node_t *n1, node_t *n2)
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

                    int32_t r2 = syntax_postfix(program, heritage2->type, response2, n2);
                    if (r2 == -1)
                    {
                        return -1;
                    }
                    else
                    if (r2 == 0)
                    {
                        syntax_error(program, heritage2->type, "reference not found");
                        return -1;
                    }
                    else
                    if (r2 == 1)
                    {
                        ilist_t *b2;
                        for (b2 = response2->begin;b2 != response2->end;b2 = b2->next)
                        {
                            node_t *item3 = (node_t *)b2->value;
                            int32_t r3 = syntax_subset(program, n1, item3);
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
syntax_eqaul_fscs(program_t *program, node_t *nfs1, node_t *ncs2)
{
    return 1;
}

static int32_t
syntax_eqaul_cscs(program_t *program, node_t *ncs1, node_t *ncs2)
{
    if (ncs1 == NULL)
    {
        if (ncs2 == NULL)
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
        if (ncs2 == NULL)
        {
            return 0;
        }
        else
        {
            node_block_t *bcs1 = (node_block_t *)ncs1->value;
            ilist_t *a1;
            for (a1 = bcs1->list->begin;a1 != bcs1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_GENERIC)
                {
                    node_pair_t *pair1 = (node_pair_t *)item1->value;
                    if (pair1->value == NULL)
                    {
                        
                    }
                }
            }
        }
    }

    return 1;
}

static int32_t
syntax_eqaul_gsgs(program_t *program, node_t *ngs1, node_t *ngs2)
{
   if (ngs1 == NULL)
    {
        if (ngs2 == NULL)
        {
            return 1;
        }
        else
        {
            node_block_t *bps2 = (node_block_t *)ngs2->value;
            ilist_t *a2;
            for (a2 = bps2->list->begin;a2 != bps2->list->end;a2 = a2->next)
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
                    node_block_t *bps2 = (node_block_t *)ngs2->value;
                    ilist_t *a2;
                    for (a2 = bps2->list->begin;a2 != bps2->list->end;a2 = a2->next)
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
                                    
                                    int32_t r1 = syntax_postfix(program, generic1->type, response1, item1);
                                    if (r1 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r1 == 0)
                                    {
                                        syntax_error(program, generic1->type, "reference not found");
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
                                            
                                            int32_t r2 = syntax_postfix(program, generic2->type, response2, item2);
                                            if (r2 == -1)
                                            {
                                                return -1;
                                            }
                                            else
                                            if (r2 == 0)
                                            {
                                                syntax_error(program, generic2->type, "reference not found");
                                                return -1;
                                            }
                                            else
                                            if (r2 == 1)
                                            {
                                                ilist_t *b2;
                                                for (b2 = response2->begin;b2 != response2->end;b2 = b2->next)
                                                {
                                                    node_t *item4 = (node_t *)b2->value;
                                                    int32_t r3 = syntax_subset(program, item3, item4);
                                                    if (r3 == -1)
                                                    {
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 0)
                                                    {
                                                        r3 = syntax_subset(program, item4, item3);
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
            node_block_t *bps2 = (node_block_t *)ngs2->value;
            ilist_t *a2;
            for (a2 = bps2->list->begin;a2 != bps2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->kind == NODE_KIND_GENERIC)
                {
                    cnt2 += 1;
                    if (cnt1 > cnt2)
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
                    node_block_t *bps2 = (node_block_t *)ngs2->value;
                    ilist_t *a2;
                    for (a2 = bps2->list->begin;a2 != bps2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_GENERIC)
                        {
                            node_generic_t *generic2 = (node_generic_t *)item2->value;
                            if (syntax_idcmp(generic1->key, generic2->key) == 1)
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

                                        int32_t r1 = syntax_postfix(program, generic1->type, response1, item1);
                                        if (r1 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r1 == 0)
                                        {
                                            syntax_error(program, generic1->type, "reference not found");
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
                                                
                                                int32_t r2 = syntax_postfix(program, generic2->type, response2, item2);
                                                if (r2 == -1)
                                                {
                                                    return -1;
                                                }
                                                else
                                                if (r2 == 0)
                                                {
                                                    syntax_error(program, generic2->type, "reference not found");
                                                    return -1;
                                                }
                                                else
                                                if (r2 == 1)
                                                {
                                                    ilist_t *b2;
                                                    for (b2 = response2->begin;b2 != response2->end;b2 = b2->next)
                                                    {
                                                        node_t *item4 = (node_t *)b2->value;
                                                        
                                                        int32_t r3 = syntax_subset(program, item3, item4);
                                                        if (r3 == -1)
                                                        {
                                                            return -1;
                                                        }
                                                        else
                                                        if (r3 == 0)
                                                        {
                                                            r3 = syntax_subset(program, item4, item3);
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

            bps2 = (node_block_t *)ngs2->value;
            for (a2 = bps2->list->begin;a2 != bps2->list->end;a2 = a2->next)
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

                            if (syntax_idcmp(generic1->key, generic2->key) == 1)
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

                                        int32_t r1 = syntax_postfix(program, generic1->type, response1, item1);
                                        if (r1 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r1 == 0)
                                        {
                                            syntax_error(program, generic1->type, "reference not found");
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
                                                
                                                int32_t r2 = syntax_postfix(program, generic2->type, response2, item2);
                                                if (r2 == -1)
                                                {
                                                    return -1;
                                                }
                                                else
                                                if (r2 == 0)
                                                {
                                                    syntax_error(program, generic2->type, "reference not found");
                                                    return -1;
                                                }
                                                else
                                                if (r2 == 1)
                                                {
                                                    ilist_t *b2;
                                                    for (b2 = response2->begin;b2 != response2->end;b2 = b2->next)
                                                    {
                                                        node_t *item4 = (node_t *)b2->value;
                                                        
                                                        int32_t r3 = syntax_subset(program, item3, item4);
                                                        if (r3 == -1)
                                                        {
                                                            return -1;
                                                        }
                                                        else
                                                        if (r3 == 0)
                                                        {
                                                            r3 = syntax_subset(program, item4, item3);
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
syntax_eqaul_gscs(program_t *program, node_t *ngs1, node_t *ncs2)
{
    return 1;
}

static int32_t
syntax_eqaul_psps(program_t *program, node_t *nps1, node_t *nps2)
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
                            
                            int32_t r1 = syntax_postfix(program, parameter1->type, response1, item1);
                            if (r1 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r1 == 0)
                            {
                                syntax_error(program, parameter1->type, "reference not found");
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
                                    
                                    int32_t r2 = syntax_postfix(program, parameter2->type, response2, item2);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        syntax_error(program, parameter2->type, "reference not found");
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
                    if (cnt1 > cnt2)
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
                            if (syntax_idcmp(parameter1->key, parameter2->key) == 1)
                            {
                                found = 1;
                                list_t *response1 = list_create();
                                if (response1 == NULL)
                                {
                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                
                                int32_t r1 = syntax_postfix(program, parameter1->type, response1, item1);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 0)
                                {
                                    syntax_error(program, parameter1->type, "reference not found");
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
                                        
                                        int32_t r2 = syntax_postfix(program, parameter2->type, response2, item2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            syntax_error(program, parameter2->type, "reference not found");
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

                            if (syntax_idcmp(parameter1->key, parameter2->key) == 1)
                            {
                                found = 1;
                                list_t *response1 = list_create();
                                if (response1 == NULL)
                                {
                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                
                                int32_t r1 = syntax_postfix(program, parameter1->type, response1, item1);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 0)
                                {
                                    syntax_error(program, parameter1->type, "reference not found");
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
                                        
                                        int32_t r2 = syntax_postfix(program, parameter2->type, response2, item2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            syntax_error(program, parameter2->type, "reference not found");
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
syntax_eqaul_asas(program_t *program, node_t *nas1, node_t *nas2)
{
    return 1;
}

static int32_t
syntax_eqaul_psas(program_t *program, node_t *nps1, node_t *nas2)
{
    return 1;
}

static int32_t
syntax_eqaul_gsfs(program_t *program, node_t *ngs1, node_t *nfs2)
{
    return 1;
}

static int32_t
syntax_eqaul_fsfs(program_t *program, node_t *nfs1, node_t *nfs2)
{
    return 1;
}


static int32_t
syntax_select(program_t *program, node_t *root, char *name, list_t *response, node_t *applicant)
{
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
                if (syntax_idstrcmp(class2->key, name) == 1)
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
            if (item1->kind == NODE_KIND_ENUM)
            {
                node_enum_t *enum1 = (node_enum_t *)item1->value;
                if (syntax_idstrcmp(enum1->key, name) == 1)
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
                    if (syntax_idstrcmp(generic3->key, name) == 1)
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

        if ((class1->heritages != NULL) && (root->id != applicant->id))
        {
            node_t *base = class1->heritages;

            list_t *repository1 = list_create();
            if (repository1 == NULL)
            {
                fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                return -1;
            }

            while (base != NULL)
            {

                node_block_t *block2 = (node_block_t *)base->value;

                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;

                    if (item2->kind == NODE_KIND_HERITAGE)
                    {
                        node_heritage_t *heritage3 = (node_heritage_t *)item2->value;
                        
                        list_t *response2 = list_create();
                        if (response2 == NULL)
                        {
                            fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }
                        
                        int32_t r1 = syntax_postfix(program, heritage3->type, response2, root);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 0)
                        {
                            syntax_error(program, heritage3->type, "reference not found");
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
                                    node_class_t *class4 = (node_class_t *)item3->value;
                                    node_t *node4 = class4->block;
                                    node_block_t *block4 = (node_block_t *)node4->value;

                                    ilist_t *a4;
                                    for (a4 = block4->list->begin;a4 != block4->list->end;a4 = a4->next)
                                    {
                                        node_t *item4 = (node_t *)a4->value;
                                        if (item4->kind == NODE_KIND_CLASS)
                                        {
                                            node_class_t *class5 = (node_class_t *)item4->value;
                                            if (syntax_idstrcmp(class5->key, name) == 1)
                                            {
                                                ilist_t *r1 = list_rpush(response, item4);
                                                if (r1 == NULL)
                                                {
                                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                            }
                                        }
                                        else
                                        if (item4->kind == NODE_KIND_ENUM)
                                        {
                                            node_enum_t *enum1 = (node_enum_t *)item4->value;
                                            if (syntax_idstrcmp(enum1->key, name) == 1)
                                            {
                                                ilist_t *r1 = list_rpush(response, item4);
                                                if (r1 == NULL)
                                                {
                                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                            }
                                        }
                                    }

                                    ilist_t *r2 = list_rpush(repository1, class4->heritages);
                                    if (r2 == NULL)
                                    {
                                        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                }
                            }
                        }

                        list_destroy(response2);
                    }
                }

                ilist_t *r1 = list_rpop(repository1);
                if (r1 != NULL)
                {
                    base = (node_t *)r1->value;
                    continue;
                }
                else
                {
                    base = NULL;
                    break;
                }

            }

            list_destroy(repository1);
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
                node_t *node2 = import1->fields;
                node_block_t *block2 = (node_block_t *)node2->value;

                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    if (item2->kind == NODE_KIND_FIELD)
                    {
                        node_pair_t *field2 = (node_pair_t *)item2->value;
                        node_t *key1 = field2->key;
                        if (key1->kind == NODE_KIND_ID)
                        {
                            if (syntax_idstrcmp(key1, name) == 1)
                            {
                                ilist_t *r1 = list_rpush(response, item2);
                                if (r1 == NULL)
                                {
                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                            }
                        }
                        else
                        if (key1->kind == NODE_KIND_PSEUDONYM)
                        {
                            node_carrier_t *carrier1 = (node_carrier_t *)key1->value;
                            node_t *base1 = carrier1->base;
                            if (base1->kind == NODE_KIND_ID)
                            {
                                if (syntax_idstrcmp(base1, name) == 1)
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
            else
            if (item1->kind == NODE_KIND_CLASS)
            {
                node_class_t *class2 = (node_class_t *)item1->value;
                if (syntax_idstrcmp(class2->key, name) == 1)
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
            if (item1->kind == NODE_KIND_ENUM)
            {
                node_enum_t *enum1 = (node_enum_t *)item1->value;
                if (syntax_idstrcmp(enum1->key, name) == 1)
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

    if (root->parent != NULL)
    {
        return syntax_select(program, root->parent, name, response, applicant);
    }
    
    if (list_count(response) > 0)
    {
        return 1;
    }

    return 0;
}

static int32_t
syntax_id(program_t *program, node_t *node, list_t *response, node_t *applicant)
{
    node_basic_t *nb1 = (node_basic_t *)node->value;
    return syntax_select(program, node->parent, nb1->value, response, applicant);
}

static int32_t
syntax_array(program_t *program, node_t *node, list_t *response, node_t *applicant)
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
syntax_object(program_t *program, node_t *node, list_t *response, node_t *applicant)
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
syntax_primary(program_t *program, node_t *node, list_t *response, node_t *applicant)
{
    if (node->kind == NODE_KIND_ID)
    {
        return syntax_id(program, node, response, applicant);
    }
    else
    if (node->kind == NODE_KIND_ARRAY)
    {
        return syntax_array(program, node, response, applicant);
    }
    else
    if (node->kind == NODE_KIND_OBJECT)
    {
        return syntax_object(program, node, response, applicant);
    }
    else
    {
        syntax_error(program, node, "not a type");
        return -1;
    }
}

static int32_t
syntax_pseudonym(program_t *program, node_t *node, list_t *response, node_t *applicant)
{
    node_carrier_t *carrier = (node_carrier_t *)node->value;
    
    list_t *response1 = list_create();
    if (response1 == NULL)
    {
        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    int32_t r1 = syntax_postfix(program, carrier->base, response1, node);
    if (r1 == -1)
    {
        return -1;
    }
    else
    if (r1 == 0)
    {
        syntax_error(program, carrier->base, "reference not found");
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
                node_t *nas2 = carrier->data;
                int32_t r1 = syntax_eqaul_gscs(program, ngs1, nas2);
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
            if (item1->kind == NODE_KIND_FIELD)
            {
                node_pair_t *field2 = (node_pair_t *)item1->value;
                if (field2->value != NULL)
                {
                    node_t *value1 = field2->value;
                    if (value1->kind == NODE_KIND_PSEUDONYM)
                    {
                        node_carrier_t *carrier2 = (node_carrier_t *)value1->value;
                        node_t *nfs1 = carrier2->data;
                        node_t *ncs2 = carrier->data;
                        int32_t r1 = syntax_eqaul_fscs(program, nfs1, ncs2);
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
                }
                else
                {
                    node_t *key1 = field2->key;
                    if (key1->kind == NODE_KIND_PSEUDONYM)
                    {
                        node_carrier_t *carrier2 = (node_carrier_t *)key1->value;
                        node_t *nfs1 = carrier2->data;
                        node_t *ncs2 = carrier->data;
                        int32_t r1 = syntax_eqaul_fscs(program, nfs1, ncs2);
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
                }
            }
            else
            {
                syntax_error(program, item1, "not a class, in (%lld:%lld)",
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
syntax_attribute(program_t *program, node_t *node, list_t *response, node_t *applicant)
{
    node_binary_t *basic = (node_binary_t *)node->value;
    
    list_t *response1 = list_create();
    if (response1 == NULL)
    {
        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    int32_t r1 = syntax_postfix(program, basic->left, response1, node);
    if (r1 == -1)
    {
        return -1;
    }
    else
    if (r1 == 0)
    {
        syntax_error(program, basic->left, "reference not found");
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
                node_t *node2 = class2->block;
                node_block_t *block2 = (node_block_t *)node2->value;
                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    if (item2->kind == NODE_KIND_CLASS)
                    {
                        node_class_t *class2 = (node_class_t *)item2->value;
                        if (syntax_idcmp(basic->right, class2->key) == 1)
                        {
                            if ((class2->flag & PARSER_MODIFIER_EXPORT) != PARSER_MODIFIER_EXPORT)
                            {
                                syntax_error(program, item2, "private access, in (%lld:%lld)",
                                    node->position.line, node->position.column);
                                return -1;
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
                        node_enum_t *enum1 = (node_enum_t *)item2->value;
                        if (syntax_idcmp(basic->right, enum1->key) == 1)
                        {
                            if ((enum1->flag & PARSER_MODIFIER_EXPORT) != PARSER_MODIFIER_EXPORT)
                            {
                                syntax_error(program, item2, "private access, in (%lld:%lld)",
                                    node->position.line, node->position.column);
                                return -1;
                            }
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
                        node_block_t *block3 = (node_block_t *)root->value;

                        ilist_t *a3;
                        for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                        {
                            node_t *item3 = (node_t *)a3->value;
                            
                            if (item3->kind == NODE_KIND_HERITAGE)
                            {
                                node_heritage_t *heritage4 = (node_heritage_t *)item3->value;
                                
                                list_t *response4 = list_create();
                                if (response4 == NULL)
                                {
                                    fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                
                                int32_t r4 = syntax_postfix(program, heritage4->type, response4, node);
                                if (r4 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r4 == 0)
                                {
                                    syntax_error(program, heritage4->type, "reference not found");
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
                                                    if (syntax_idcmp(class6->key, basic->right) == 1)
                                                    {
                                                        if ((class6->flag & PARSER_MODIFIER_EXPORT) != PARSER_MODIFIER_EXPORT)
                                                        {
                                                            syntax_error(program, item6, "private access, in (%lld:%lld)",
                                                                node->position.line, node->position.column);
                                                            return -1;
                                                        }

                                                        if ((class6->flag & PARSER_MODIFIER_PROTECT) == PARSER_MODIFIER_PROTECT)
                                                        {
                                                            syntax_error(program, item6, "protect access, in (%lld:%lld)",
                                                                node->position.line, node->position.column);
                                                            return -1;
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
                                                    if (syntax_idcmp(enum1->key, basic->right) == 1)
                                                    {
                                                        if ((enum1->flag & PARSER_MODIFIER_EXPORT) != PARSER_MODIFIER_EXPORT)
                                                        {
                                                            syntax_error(program, item6, "private access, in (%lld:%lld)",
                                                                node->position.line, node->position.column);
                                                            return -1;
                                                        }

                                                        if ((enum1->flag & PARSER_MODIFIER_PROTECT) == PARSER_MODIFIER_PROTECT)
                                                        {
                                                            syntax_error(program, item6, "protect access, in (%lld:%lld)",
                                                                node->position.line, node->position.column);
                                                            return -1;
                                                        }

                                                        ilist_t *r1 = list_rpush(response, item6);
                                                        if (r1 == NULL)
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
                                    }
                                }

                                list_destroy(response4);
                            }
                        }
                    
                        ilist_t *r1 = list_rpop(repository1);
                        if (r1 != NULL)
                        {
                            root = (node_t *)r1->value;
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
                syntax_error(program, item1, "not a class, in (%lld:%lld)",
                    node->position.line, node->position.column);
                return -1;
            }
        }
    }

    list_destroy(response1);

    return 0;
}

static int32_t
syntax_postfix(program_t *program, node_t *node, list_t *response, node_t *applicant)
{
    if (node->kind == NODE_KIND_ATTRIBUTE)
    {
        return syntax_attribute(program, node, response, applicant);
    }
    else
    if (node->kind == NODE_KIND_PSEUDONYM)
    {
        return syntax_pseudonym(program, node, response, applicant);
    }
    else
    {
        return syntax_primary(program, node, response, applicant);
    }
    
}



static int32_t
syntax_if(program_t *program, node_t *node)
{
    node_if_t *if1 = (node_if_t *)node->value;

    if (if1->key != NULL)
    {
        node_t *sub = node;
        node_t *current = node->parent;
        while (current != NULL)
        {
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

                    if (item2->kind == NODE_KIND_IF)
                    {
                        node_if_t *if2 = (node_if_t *)item2->value;
                        if (if2->key != NULL)
                        {
                            if (syntax_idcmp(if1->key, if2->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FOR)
                    {
                        node_for_t *for2 = (node_for_t *)item2->value;
                        if (for2->key != NULL)
                        {
                            if (syntax_idcmp(if1->key, for2->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
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
                            if (syntax_idcmp(if1->key, for2->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
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
                            if (syntax_idcmp(if1->key, var2->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
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
                                    if (syntax_idcmp(if1->key, property1->key) == 1)
                                    {
                                        syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                            item3->position.line, item3->position.column);
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
            if (current->kind == NODE_KIND_FUNC)
            {
                node_func_t *func2 = (node_func_t *)current->value;
                if (func2->generics != NULL)
                {
                    node_t *node2 = func2->generics;
                    node_block_t *block2 = (node_block_t *)node2->value;

                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->id == node->id)
                        {
                            break;
                        }

                        if (item2->kind == NODE_KIND_GENERIC)
                        {
                            node_generic_t *generic3 = (node_generic_t *)item2->value;
                            if (syntax_idcmp(if1->key, generic3->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
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
                        if (item3->id == node->id)
                        {
                            break;
                        }

                        if (item3->kind == NODE_KIND_PARAMETER)
                        {
                            node_parameter_t *parameter3 = (node_parameter_t *)item3->value;
                            if (syntax_idcmp(if1->key, parameter3->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item3->position.line, item3->position.column);
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
            }
        }
    }

	return 1;
}

static int32_t
syntax_for(program_t *program, node_t *node)
{
    node_for_t *for1 = (node_for_t *)node->value;

    if (for1->key != NULL)
    {
        node_t *sub = node;
        node_t *current = node->parent;
        while (current != NULL)
        {
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

                    if (item2->kind == NODE_KIND_IF)
                    {
                        node_if_t *if2 = (node_if_t *)item2->value;
                        if (if2->key != NULL)
                        {
                            if (syntax_idcmp(for1->key, if2->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FOR)
                    {
                        node_for_t *for2 = (node_for_t *)item2->value;
                        if (for2->key != NULL)
                        {
                            if (syntax_idcmp(for1->key, for2->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
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
                            if (syntax_idcmp(for1->key, for2->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
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
                            if (syntax_idcmp(for1->key, var2->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
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
                                    if (syntax_idcmp(for1->key, property1->key) == 1)
                                    {
                                        syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                            item3->position.line, item3->position.column);
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
            if (current->kind == NODE_KIND_FUNC)
            {
                node_func_t *func2 = (node_func_t *)current->value;

                if (func2->generics != NULL)
                {
                    node_t *node2 = func2->generics;
                    node_block_t *block2 = (node_block_t *)node2->value;

                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->id == node->id)
                        {
                            break;
                        }

                        if (item2->kind == NODE_KIND_GENERIC)
                        {
                            node_generic_t *generic3 = (node_generic_t *)item2->value;
                            if (syntax_idcmp(for1->key, generic3->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
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
                        if (item3->id == node->id)
                        {
                            break;
                        }

                        if (item3->kind == NODE_KIND_PARAMETER)
                        {
                            node_parameter_t *parameter3 = (node_parameter_t *)item3->value;
                            if (syntax_idcmp(for1->key, parameter3->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item3->position.line, item3->position.column);
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
            }
        }
    }
	return 1;
}

static int32_t
syntax_forin(program_t *program, node_t *node)
{
    node_forin_t *for1 = (node_forin_t *)node->value;

    if (for1->key != NULL)
    {
        node_t *sub = node;
        node_t *current = node->parent;
        while (current != NULL)
        {
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

                    if (item2->kind == NODE_KIND_IF)
                    {
                        node_if_t *if2 = (node_if_t *)item2->value;
                        if (if2->key != NULL)
                        {
                            if (syntax_idcmp(for1->key, if2->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item2->kind == NODE_KIND_FOR)
                    {
                        node_for_t *for2 = (node_for_t *)item2->value;
                        if (for2->key != NULL)
                        {
                            if (syntax_idcmp(for1->key, for2->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
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
                            if (syntax_idcmp(for1->key, for2->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
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
                            if (syntax_idcmp(for1->key, var2->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
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
                                    if (syntax_idcmp(for1->key, property1->key) == 1)
                                    {
                                        syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                            item3->position.line, item3->position.column);
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
            if (current->kind == NODE_KIND_FUNC)
            {
                node_func_t *func2 = (node_func_t *)current->value;

                if (func2->generics != NULL)
                {
                    node_t *node2 = func2->generics;
                    node_block_t *block2 = (node_block_t *)node2->value;

                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->id == node->id)
                        {
                            break;
                        }

                        if (item2->kind == NODE_KIND_GENERIC)
                        {
                            node_generic_t *generic3 = (node_generic_t *)item2->value;
                            if (syntax_idcmp(for1->key, generic3->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
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
                        if (item3->id == node->id)
                        {
                            break;
                        }

                        if (item3->kind == NODE_KIND_PARAMETER)
                        {
                            node_parameter_t *parameter3 = (node_parameter_t *)item3->value;
                            if (syntax_idcmp(for1->key, parameter3->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item3->position.line, item3->position.column);
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
            }
        }
    }
    return 1;
}

static int32_t
syntax_parameter(program_t *program, node_t *node)
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
                    if (syntax_idcmp(parameter1->key, parameter3->key) == 1)
                    {
                        syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                            item2->position.line, item2->position.column);
                        return -1;
                    }
                }
            }
            
            sub = current;
            current = current->parent;
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
                    if (item3->id == node->id)
                    {
                        break;
                    }

                    if (item3->kind == NODE_KIND_GENERIC)
                    {
                        node_generic_t *generic3 = (node_generic_t *)item3->value;
                        if (syntax_idcmp(parameter1->key, generic3->key) == 1)
                        {
                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                item3->position.line, item3->position.column);
                            return -1;
                        }
                    }
                }
            }
            break;
        }
        else
        if (current->kind == NODE_KIND_FUNC)
        {
            node_func_t *func2 = (node_func_t *)current->value;

            if (func2->generics != NULL)
            {
                node_t *node2 = func2->generics;
                node_block_t *block3 = (node_block_t *)node2->value;

                ilist_t *a3;
                for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                {
                    node_t *item3 = (node_t *)a3->value;
                    if (item3->id == node->id)
                    {
                        break;
                    }

                    if (item3->kind == NODE_KIND_GENERIC)
                    {
                        node_generic_t *generic3 = (node_generic_t *)item3->value;
                        if (syntax_idcmp(parameter1->key, generic3->key) == 1)
                        {
                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                item3->position.line, item3->position.column);
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
        }
    }

    list_t *response1 = list_create();
    if (response1 == NULL)
    {
        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    int32_t r1 = syntax_postfix(program, parameter1->type, response1, node);
    if (r1 == -1)
    {
        return -1;
    }
    else
    if (r1 == 0)
    {
        syntax_error(program, parameter1->type, "reference not found");
        return -1;
    }

    list_destroy(response1);

    return 1;
}

static int32_t
syntax_parameters(program_t *program, node_t *node)
{
	node_block_t *parameters = (node_block_t *)node->value;
    ilist_t *a1;
    for (a1 = parameters->list->begin;a1 != parameters->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        int32_t result;
        result = syntax_parameter(program, item1);
        if (result == -1)
        {
            return -1;
        }
    }
	return 1;
}

static int32_t
syntax_catch(program_t *program, node_t *node)
{
    return 1;
}

static int32_t
syntax_try(program_t *program, node_t *node)
{
	return 1;
}

static int32_t
syntax_var(program_t *program, node_t *node)
{
    node_var_t *var1 = (node_var_t *)node->value;

    node_t *sub = node;
    node_t *current = node->parent;
    while (current != NULL)
    {
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

                if (item2->kind == NODE_KIND_IF)
                {
                    node_if_t *if2 = (node_if_t *)item2->value;
                    if (if2->key != NULL)
                    {
                        if (var1->key->kind == NODE_KIND_ID)
                        {
                            if (syntax_idcmp(var1->key, if2->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
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
                                    if (syntax_idcmp(property1->key, if2->key) == 1)
                                    {
                                        syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                            item3->position.line, item3->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_FOR)
                {
                    node_for_t *for2 = (node_for_t *)item2->value;
                    if (for2->key != NULL)
                    {
                        if (var1->key->kind == NODE_KIND_ID)
                        {
                            if (syntax_idcmp(var1->key, for2->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
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
                                    if (syntax_idcmp(property1->key, for2->key) == 1)
                                    {
                                        syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                            item3->position.line, item3->position.column);
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
                            if (syntax_idcmp(var1->key, for2->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
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
                                    if (syntax_idcmp(property1->key, for2->key) == 1)
                                    {
                                        syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                            item3->position.line, item3->position.column);
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
                            if (syntax_idcmp(var1->key, var2->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
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
                                    if (syntax_idcmp(property1->key, var2->key) == 1)
                                    {
                                        syntax_error(program, property1->key, "already defined, previous in (%lld:%lld)",
                                            item2->position.line, item2->position.column);
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
                                    if (syntax_idcmp(var1->key, property1->key) == 1)
                                    {
                                        syntax_error(program, var1->key, "already defined, previous in (%lld:%lld)",
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
                                            if (syntax_idcmp(property1->key, property2->key) == 1)
                                            {
                                                syntax_error(program, property1->key, "already defined, previous in (%lld:%lld)",
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
        }
        else
        if (current->kind == NODE_KIND_FUNC)
        {
            node_func_t *func2 = (node_func_t *)current->value;

            if (func2->generics != NULL)
            {
                node_t *node2 = func2->generics;
                node_block_t *block2 = (node_block_t *)node2->value;

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
                        if (var1->key->kind == NODE_KIND_ID)
                        {
                            if (syntax_idcmp(var1->key, generic3->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
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
                                    if (syntax_idcmp(property1->key, generic3->key) == 1)
                                    {
                                        syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                            item3->position.line, item3->position.column);
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
                    if (item3->id == sub->id)
                    {
                        break;
                    }

                    if (item3->kind == NODE_KIND_PARAMETER)
                    {
                        node_parameter_t *parameter3 = (node_parameter_t *)item3->value;
                        if (var1->key->kind == NODE_KIND_ID)
                        {
                            if (syntax_idcmp(var1->key, parameter3->key) == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item3->position.line, item3->position.column);
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
                                    if (syntax_idcmp(property1->key, parameter3->key) == 1)
                                    {
                                        syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                            item4->position.line, item4->position.column);
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
        }
    }

	return 1;
}

static int32_t
syntax_statement(program_t *program, node_t *node)
{
    if (node->kind == NODE_KIND_IF)
    {
        int32_t result;
        result = syntax_if(program, node);
        if (result == -1)
        {
            return -1;
        }
    }
    else 
    if (node->kind == NODE_KIND_FOR)
    {
        int32_t result;
        result = syntax_for(program, node);
        if (result == -1)
        {
            return -1;
        }
    }
    else
    if (node->kind == NODE_KIND_FORIN)
    {
        int32_t result;
        result = syntax_forin(program, node);
        if (result == -1)
        {
            return -1;
        }
    }
    else
    if (node->kind == NODE_KIND_TRY)
    {
        int32_t result;
        result = syntax_try(program, node);
        if (result == -1)
        {
            return -1;
        }
    }
    else
    if (node->kind == NODE_KIND_VAR)
    {
        int32_t result;
        result = syntax_var(program, node);
        if (result == -1)
        {
            return -1;
        }
    }
    return 1;
}

static int32_t
syntax_body(program_t *program, node_t *node)
{
    node_block_t *block1 = (node_block_t *)node->value;
    ilist_t *a1;
    for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
    {
        node_t *item = (node_t *)a1->value;
        int32_t result = syntax_statement(program, item);
        if (result == -1)
        {
            return -1;
        }
    }

    return 1;
}

static int32_t
syntax_generic(program_t *program, node_t *node)
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
                    if (syntax_idcmp(generic1->key, generic3->key) == 1)
                    {
                        syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                            item2->position.line, item2->position.column);
                        return -1;
                    }
                }
            }
            
            sub = current;
            current = current->parent;
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
                    if (syntax_idcmp(generic1->key, class3->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = class3->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
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
                                if (item3->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *func3 = (node_func_t *)item3->value;
                                    if (syntax_idstrcmp(func3->key, "Constructor") == 1)
                                    {
                                        node_t *nps1 = NULL;
                                        node_t *nps2 = func3->parameters;
                                        int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            syntax_error(program, item2, "already defined, previous in (%lld:%lld)",
                                                node->position.line, node->position.column);
                                            return -1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_FUNC)
                {
                    node_func_t *func2 = (node_func_t *)item2->value;

                    if (syntax_idcmp(generic1->key, func2->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = func2->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *nps1 = NULL;
                            node_t *nps2 = func2->parameters;
                            int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                syntax_error(program, item2, "already defined, previous in (%lld:%lld)",
                                    node->position.line, node->position.column);
                                return -1;
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_ENUM)
                {
                    node_enum_t *enum2 = (node_enum_t *)item2->value;

                    if (syntax_idcmp(generic1->key, enum2->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = NULL;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *nps1 = NULL;
                            node_t *nps2 = NULL;
                            int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                syntax_error(program, item2, "already defined, previous in (%lld:%lld)",
                                    node->position.line, node->position.column);
                                return -1;
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_PROPERTY)
                {
                    node_property_t *property2 = (node_property_t *)item2->value;

                    if (syntax_idcmp(generic1->key, property2->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = NULL;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *nps1 = NULL;
                            node_t *nps2 = NULL;
                            int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                syntax_error(program, item2, "already defined, previous in (%lld:%lld)",
                                    node->position.line, node->position.column);
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

        int32_t r1 = syntax_postfix(program, generic1->type, response1, node);
        if (r1 == -1)
        {
            return -1;
        }
        else
        if (r1 == 0)
        {
            syntax_error(program, generic1->type, "reference not found");
            return -1;
        }

        list_destroy(response1);
    }

    return 1;
}

static int32_t
syntax_generics(program_t *program, node_t *node)
{
	node_block_t *generics = (node_block_t *)node->value;
    ilist_t *a1;
    for (a1 = generics->list->begin;a1 != generics->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        int32_t result;
        result = syntax_generic(program, item1);
        if (result == -1)
        {
            return -1;
        }
    }
	return 1;
}

static int32_t
syntax_func(program_t *program, node_t *node)
{
	node_func_t *func1 = (node_func_t *)node->value;

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
                    if (syntax_idcmp(func1->key, class3->key) == 1)
                    {
                        node_t *ngs1 = func1->generics;
                        node_t *ngs2 = class3->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
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
                                if (item3->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *func3 = (node_func_t *)item3->value;
                                    if (syntax_idstrcmp(func3->key, "Constructor") == 1)
                                    {
                                        node_t *nps1 = func1->parameters;
                                        node_t *nps2 = func3->parameters;
                                        int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                                item2->position.line, item2->position.column);
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

                    if (syntax_idcmp(func1->key, enum2->key) == 1)
                    {
                        node_t *ngs1 = func1->generics;
                        node_t *ngs2 = NULL;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *nps1 = func1->parameters;
                            node_t *nps2 = NULL;
                            int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
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

                if (item2->id == node->id)
                {
                    break;
                }

                if (item2->kind == NODE_KIND_CLASS)
                {
                    node_class_t *class3 = (node_class_t *)item2->value;
                    if (syntax_idcmp(func1->key, class3->key) == 1)
                    {
                        node_t *ngs1 = func1->generics;
                        node_t *ngs2 = class3->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
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
                                if (item3->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *func3 = (node_func_t *)item3->value;
                                    if (syntax_idstrcmp(func3->key, "Constructor") == 1)
                                    {
                                        node_t *nps1 = func1->parameters;
                                        node_t *nps2 = func3->parameters;
                                        int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                                item2->position.line, item2->position.column);
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

                    if (syntax_idcmp(func1->key, enum2->key) == 1)
                    {
                        node_t *ngs1 = func1->generics;
                        node_t *ngs2 = NULL;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *nps1 = func1->parameters;
                            node_t *nps2 = NULL;
                            int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_FUNC)
                {
                    node_func_t *func2 = (node_func_t *)item2->value;

                    if (syntax_idcmp(func1->key, func2->key) == 1)
                    {
                        node_t *ngs1 = func1->generics;
                        node_t *ngs2 = func2->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *nps1 = func1->parameters;
                            node_t *nps2 = func2->parameters;
                            int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_PROPERTY)
                {
                    node_property_t *property2 = (node_property_t *)item2->value;

                    if (syntax_idcmp(func1->key, property2->key) == 1)
                    {
                        node_t *ngs1 = func1->generics;
                        node_t *ngs2 = NULL;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *nps1 = func1->parameters;
                            node_t *nps2 = NULL;
                            int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
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
        }
    }

    if (func1->generics != NULL)
    {
        int32_t r1 = syntax_generics(program, func1->generics);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (func1->parameters != NULL)
    {
        int32_t r1 = syntax_parameters(program, func1->parameters);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (func1->body != NULL)
    {
        int32_t r1 = syntax_body(program, func1->body);
        if (r1 == -1)
        {
            return -1;
        }
    }

	return 1;
}

static int32_t
syntax_enum(program_t *program, node_t *node)
{
	node_enum_t *enum1 = (node_enum_t *)node->value;
    node_t *node1 = enum1->block;
    node_block_t *block1 = (node_block_t *)node1->value;
    
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
                    node_t *fields1 = import1->fields;
                    node_block_t *block3 = (node_block_t *)fields1->value;
                    ilist_t *a3;
                    for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                    {
                        node_t *item3 = (node_t *)a3->value;
                        if (item3->kind == NODE_KIND_FIELD)
                        {
                            node_pair_t *field1 = (node_pair_t *)item3->value;
                            node_t *key1 = field1->key;
                            if (key1->kind == NODE_KIND_ID)
                            {
                                if (syntax_idcmp(key1, enum1->key) == 1)
                                {
                                    syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                        key1->position.line, key1->position.column);
                                    return -1;
                                }
                            }
                            else
                            if (key1->kind == NODE_KIND_PSEUDONYM)
                            {
                                node_carrier_t *carrier1 = (node_carrier_t *)key1->value;
                                node_t *key2 = carrier1->base;
                                if (key2->kind == NODE_KIND_ID)
                                {
                                    if (syntax_idcmp(key2, enum1->key) == 1)
                                    {
                                        node_t *ngs1 = NULL;
                                        node_t *nfs2 = carrier1->data;
                                        int32_t r1 = syntax_eqaul_gsfs(program, ngs1, nfs2);
                                        if (r1 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r1 == 1)
                                        {
                                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                                key1->position.line, key1->position.column);
                                            return -1;
                                        }
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
                    if (syntax_idcmp(enum1->key, class3->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = class3->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
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
                                if (item3->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *func3 = (node_func_t *)item3->value;
                                    if (syntax_idstrcmp(func3->key, "Constructor") == 1)
                                    {
                                        node_t *nps1 = NULL;
                                        node_t *nps2 = func3->parameters;
                                        int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                                item2->position.line, item2->position.column);
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

                    if (syntax_idcmp(enum1->key, enum2->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = NULL;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *nps1 = NULL;
                            node_t *nps2 = NULL;
                            int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
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

                if (item2->id == node->id)
                {
                    break;
                }

                if (item2->kind == NODE_KIND_CLASS)
                {
                    node_class_t *class3 = (node_class_t *)item2->value;
                    if (syntax_idcmp(enum1->key, class3->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = class3->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
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
                                if (item3->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *func3 = (node_func_t *)item3->value;
                                    if (syntax_idstrcmp(func3->key, "Constructor") == 1)
                                    {
                                        node_t *nps1 = NULL;
                                        node_t *nps2 = func3->parameters;
                                        int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                                item2->position.line, item2->position.column);
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

                    if (syntax_idcmp(enum1->key, enum2->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = NULL;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *nps1 = NULL;
                            node_t *nps2 = NULL;
                            int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_FUNC)
                {
                    node_func_t *func2 = (node_func_t *)item2->value;

                    if (syntax_idcmp(enum1->key, func2->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = func2->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *nps1 = NULL;
                            node_t *nps2 = func2->parameters;
                            int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
                                return -1;
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_PROPERTY)
                {
                    node_property_t *property2 = (node_property_t *)item2->value;

                    if (syntax_idcmp(enum1->key, property2->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = NULL;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *nps1 = NULL;
                            node_t *nps2 = NULL;
                            int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                    item2->position.line, item2->position.column);
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
            current = current->parent;
        }
    }

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
            if (syntax_idcmp(member1->key, member2->key) == 1)
            {
                syntax_error(program, item1, "already defined, previous in (%lld:%lld)",
                    item2->position.line, item2->position.column);
                return -1;
            }
        }
    }

	return 1;
}

static int32_t
syntax_field(program_t *program, node_t *node)
{
    node_pair_t *field1 = (node_pair_t *)node->value;

    if (field1->value != NULL)
    {
        
        node_t *key1 = field1->key;
        if (key1->kind == NODE_KIND_PSEUDONYM)
        {
            node_carrier_t *carrier1 = (node_carrier_t *)key1->value;
            node_t *base1 = carrier1->base;
            if (base1->kind != NODE_KIND_ID)
            {
                syntax_error(program, key1, "not an id");
                return -1;
            }
        }

        node_t *value1 = field1->value;
        while(value1 != NULL)
        {
            if (value1->kind == NODE_KIND_PSEUDONYM)
            {
                node_carrier_t *carrier1 = (node_carrier_t *)value1->value;
                value1 = carrier1->base;
            }
            else
            if (value1->kind == NODE_KIND_ATTRIBUTE)
            {
                node_binary_t *binary = (node_binary_t *)value1->value;
                value1 = binary->left;
            }
            else
            if (value1->kind == NODE_KIND_ID)
            {
                break;
            }
            else
            {
                syntax_error(program, field1->value, "not a route");
                return -1;
            }
        }
        
    }
    else
    {
        node_t *key1 = field1->key;
        if (key1->kind == NODE_KIND_PSEUDONYM)
        {
            node_carrier_t *carrier1 = (node_carrier_t *)key1->value;
            node_t *base1 = carrier1->base;
            if (base1->kind != NODE_KIND_ID)
            {
                syntax_error(program, key1, "not an id");
                return -1;
            }
        }
        else
        if (key1->kind != NODE_KIND_ID)
        {
            syntax_error(program, key1, "wrong format");
            return -1;
        }
    }
    
    node_t *sub = node;
    node_t *current = node->parent;
    while (current != NULL)
    {
        if (current->kind == NODE_KIND_FIELDS)
        {
            node_block_t *block3 = (node_block_t *)current->value;

            ilist_t *a3;
            for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
            {
                node_t *item3 = (node_t *)a3->value;
                if (item3->kind == NODE_KIND_FIELD)
                {
                    if (item3->id == sub->id)
                    {
                        break;
                    }

                    node_pair_t *field3 = (node_pair_t *)item3->value;
                    node_t *key1 = field1->key;
                    
                    if (key1->kind == NODE_KIND_ID)
                    {
                        node_t *key3 = field3->key;
                        if (key3->kind == NODE_KIND_ID)
                        {
                            if (syntax_idcmp(key1, key3) == 1)
                            {
                                syntax_error(program, key1, "already defined, previous in (%lld:%lld)",
                                    key3->position.line, key3->position.column);
                                return -1;
                            }
                        }
                        else
                        if (key3->kind == NODE_KIND_PSEUDONYM)
                        {
                            node_carrier_t *carrier3 = (node_carrier_t *)key3->value;
                            node_t *base3 = carrier3->base;
                            if (base3->kind == NODE_KIND_ID)
                            {
                                if (syntax_idcmp(key1, base3) == 1)
                                {
                                    node_t *nfs1 = NULL;
                                    node_t *nfs2 = carrier3->data;
                                    int32_t r1 = syntax_eqaul_fsfs(program, nfs1, nfs2);
                                    if (r1 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r1 == 1)
                                    {
                                        syntax_error(program, key1, "already defined, previous in (%lld:%lld)",
                                            key3->position.line, key3->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                    }
                    else
                    if (key1->kind == NODE_KIND_PSEUDONYM)
                    {
                        node_t *key3 = field3->key;
                        if (key3->kind == NODE_KIND_ID)
                        {
                            node_carrier_t *carrier1 = (node_carrier_t *)key1->value;
                            node_t *base1 = carrier1->base;
                            if (base1->kind == NODE_KIND_ID)
                            {
                                if (syntax_idcmp(base1, key3) == 1)
                                {
                                    node_t *nfs1 = NULL;
                                    node_t *nfs2 = carrier1->data;
                                    int32_t r1 = syntax_eqaul_fsfs(program, nfs1, nfs2);
                                    if (r1 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r1 == 1)
                                    {
                                        syntax_error(program, key1, "already defined, previous in (%lld:%lld)",
                                            key3->position.line, key3->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                        else
                        if (key3->kind == NODE_KIND_PSEUDONYM)
                        {
                            node_carrier_t *carrier1 = (node_carrier_t *)key1->value;
                            node_t *base1 = carrier1->base;
                            if (base1->kind == NODE_KIND_ID)
                            {
                                node_carrier_t *carrier3 = (node_carrier_t *)key3->value;
                                node_t *base3 = carrier3->base;
                                if (base3->kind == NODE_KIND_ID)
                                {
                                    if (syntax_idcmp(base1, base3) == 1)
                                    {
                                        node_t *nfs1 = carrier1->data;
                                        node_t *nfs2 = carrier3->data;
                                        int32_t r1 = syntax_eqaul_fsfs(program, nfs1, nfs2);
                                        if (r1 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r1 == 1)
                                        {
                                            syntax_error(program, key1, "already defined, previous in (%lld:%lld)",
                                                key3->position.line, key3->position.column);
                                            return -1;
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
                    node_t *node3 = import1->fields;
                    node_block_t *block3 = (node_block_t *)node3->value;

                    ilist_t *a3;
                    for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                    {
                        node_t *item3 = (node_t *)a3->value;
                        if (item3->kind == NODE_KIND_FIELD)
                        {
                            if (item3->id == node->id)
                            {
                                break;
                            }

                            node_pair_t *field3 = (node_pair_t *)item3->value;
                            node_t *key1 = field1->key;
                            if (key1->kind == NODE_KIND_ID)
                            {
                                node_t *key3 = field3->key;
                                if (key3->kind == NODE_KIND_ID)
                                {
                                    if (syntax_idcmp(key1, key3) == 1)
                                    {
                                        syntax_error(program, key1, "already defined, previous in (%lld:%lld)",
                                            key3->position.line, key3->position.column);
                                        return -1;
                                    }
                                }
                                else
                                if (key3->kind == NODE_KIND_PSEUDONYM)
                                {
                                    node_carrier_t *carrier3 = (node_carrier_t *)key3->value;
                                    node_t *base3 = carrier3->base;
                                    if (base3->kind == NODE_KIND_ID)
                                    {
                                        if (syntax_idcmp(key1, base3) == 1)
                                        {
                                            node_t *nfs1 = NULL;
                                            node_t *nfs2 = carrier3->data;
                                            int32_t r1 = syntax_eqaul_fsfs(program, nfs1, nfs2);
                                            if (r1 == -1)
                                            {
                                                return -1;
                                            }
                                            else
                                            if (r1 == 1)
                                            {
                                                syntax_error(program, key1, "already defined, previous in (%lld:%lld)",
                                                    key3->position.line, key3->position.column);
                                                return -1;
                                            }
                                        }
                                    }
                                }
                            }
                            else
                            if (key1->kind == NODE_KIND_PSEUDONYM)
                            {
                                node_t *key3 = field3->key;
                                if (key3->kind == NODE_KIND_ID)
                                {
                                    node_carrier_t *carrier1 = (node_carrier_t *)key1->value;
                                    node_t *base1 = carrier1->base;
                                    if (base1->kind == NODE_KIND_ID)
                                    {
                                        if (syntax_idcmp(base1, key3) == 1)
                                        {
                                            node_t *nfs1 = NULL;
                                            node_t *nfs2 = carrier1->data;
                                            int32_t r1 = syntax_eqaul_fsfs(program, nfs1, nfs2);
                                            if (r1 == -1)
                                            {
                                                return -1;
                                            }
                                            else
                                            if (r1 == 1)
                                            {
                                                syntax_error(program, key1, "already defined, previous in (%lld:%lld)",
                                                    key3->position.line, key3->position.column);
                                                return -1;
                                            }
                                        }
                                    }
                                }
                                else
                                if (key3->kind == NODE_KIND_PSEUDONYM)
                                {
                                    node_carrier_t *carrier1 = (node_carrier_t *)key1->value;
                                    node_t *base1 = carrier1->base;
                                    if (base1->kind == NODE_KIND_ID)
                                    {
                                        node_carrier_t *carrier3 = (node_carrier_t *)key3->value;
                                        node_t *base3 = carrier3->base;
                                        if (base3->kind == NODE_KIND_ID)
                                        {
                                            if (syntax_idcmp(base1, base3) == 1)
                                            {
                                                node_t *nfs1 = carrier1->data;
                                                node_t *nfs2 = carrier3->data;
                                                int32_t r1 = syntax_eqaul_fsfs(program, nfs1, nfs2);
                                                if (r1 == -1)
                                                {
                                                    return -1;
                                                }
                                                else
                                                if (r1 == 1)
                                                {
                                                    syntax_error(program, key1, "already defined, previous in (%lld:%lld)",
                                                        key3->position.line, key3->position.column);
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
                if (item2->kind == NODE_KIND_CLASS)
                {
                    node_class_t *class3 = (node_class_t *)item2->value;

                    node_t *key1 = field1->key;
                    if (key1->kind == NODE_KIND_ID)
                    {
                        if (syntax_idcmp(key1, class3->key) == 1)
                        {
                            node_t *ngs1 = class3->generics;
                            node_t *nfs2 = NULL;
                            int32_t r1 = syntax_eqaul_gsfs(program, ngs1, nfs2);
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
                                    if (item3->kind == NODE_KIND_FUNC)
                                    {
                                        node_func_t *func3 = (node_func_t *)item3->value;
                                        if (syntax_idstrcmp(func3->key, "Constructor") == 1)
                                        {
                                            node_t *nps1 = func3->parameters;
                                            node_t *nps2 = NULL;
                                            int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                            if (r2 == -1)
                                            {
                                                return -1;
                                            }
                                            else
                                            if (r2 == 1)
                                            {
                                                syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                                    key1->position.line, key1->position.column);
                                                return -1;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else
                    if (key1->kind == NODE_KIND_PSEUDONYM)
                    {
                        node_carrier_t *carrier1 = (node_carrier_t *)key1->value;
                        node_t *base1 = carrier1->base;
                        if (base1->kind == NODE_KIND_ID)
                        {
                            if (syntax_idcmp(base1, class3->key) == 1)
                            {
                                node_t *ngs1 = class3->generics;
                                node_t *nfs2 = carrier1->data;
                                int32_t r1 = syntax_eqaul_gsfs(program, ngs1, nfs2);
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
                                        if (item3->kind == NODE_KIND_FUNC)
                                        {
                                            node_func_t *func3 = (node_func_t *)item3->value;
                                            if (syntax_idstrcmp(func3->key, "Constructor") == 1)
                                            {
                                                node_t *nps1 = func3->parameters;
                                                node_t *nps2 = NULL;
                                                int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                                if (r2 == -1)
                                                {
                                                    return -1;
                                                }
                                                else
                                                if (r2 == 1)
                                                {
                                                    syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                                        key1->position.line, key1->position.column);
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
                if (item2->kind == NODE_KIND_ENUM)
                {
                    node_enum_t *enum2 = (node_enum_t *)item2->value;

                    node_t *key1 = field1->key;
                    if (key1->kind == NODE_KIND_ID)
                    {
                        if (syntax_idcmp(key1, enum2->key) == 1)
                        {
                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                key1->position.line, key1->position.column);
                            return -1;
                        }
                    }
                    else
                    if (key1->kind == NODE_KIND_PSEUDONYM)
                    {
                        node_carrier_t *carrier1 = (node_carrier_t *)key1->value;
                        node_t *base1 = carrier1->base;
                        if (base1->kind == NODE_KIND_ID)
                        {
                            if (syntax_idcmp(base1, enum2->key) == 1)
                            {
                                node_t *ngs1 = NULL;
                                node_t *nfs2 = carrier1->data;
                                int32_t r1 = syntax_eqaul_gsfs(program, ngs1, nfs2);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 1)
                                {
                                    syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                        key1->position.line, key1->position.column);
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
        {
            sub = current;
            current = current->parent;
        }
    }

    return 1;
}

static int32_t
syntax_fields(program_t *program, node_t *node)
{
    node_block_t *fields = (node_block_t *)node->value;
    
	ilist_t *a1;
    for (a1 = fields->list->begin;a1 != fields->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        int32_t result;
        result = syntax_field(program, item1);
        if (result == -1)
        {
            return -1;
        }
    }
	return 1;
}

static int32_t
syntax_import(program_t *program, node_t *node)
{
	node_import_t *import1 = (node_import_t *)node->value;

	if (import1->fields != NULL)
    {
        int32_t r1 = syntax_fields(program, import1->fields);
        if (r1 == -1)
        {
            return -1;
        }
    }

	return 1;
}

static int32_t
syntax_property(program_t *program, node_t *node)
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
                    if (item3->id == node->id)
                    {
                        break;
                    }

                    if (item3->kind == NODE_KIND_GENERIC)
                    {
                        node_generic_t *generic3 = (node_generic_t *)item3->value;
                        if (syntax_idcmp(property1->key, generic3->key) == 1)
                        {
                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                item3->position.line, item3->position.column);
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
                    if (item3->id == node->id)
                    {
                        break;
                    }

                    if (item3->kind == NODE_KIND_HERITAGE)
                    {
                        node_heritage_t *heritage3 = (node_heritage_t *)item3->value;
                        if (syntax_idcmp(property1->key, heritage3->key) == 1)
                        {
                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                item3->position.line, item3->position.column);
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
                    if (syntax_idcmp(property1->key, class3->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = class3->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
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
                                if (item3->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *func3 = (node_func_t *)item3->value;
                                    if (syntax_idstrcmp(func3->key, "Constructor") == 1)
                                    {
                                        node_t *nps1 = NULL;
                                        node_t *nps2 = func3->parameters;
                                        int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            syntax_error(program, item2, "already defined, previous in (%lld:%lld)",
                                                node->position.line, node->position.column);
                                            return -1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_FUNC)
                {
                    node_func_t *func2 = (node_func_t *)item2->value;

                    if (syntax_idcmp(property1->key, func2->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = func2->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *nps1 = NULL;
                            node_t *nps2 = func2->parameters;
                            int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                syntax_error(program, item2, "already defined, previous in (%lld:%lld)",
                                    node->position.line, node->position.column);
                                return -1;
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_ENUM)
                {
                    node_enum_t *enum2 = (node_enum_t *)item2->value;

                    if (syntax_idcmp(property1->key, enum2->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = NULL;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *nps1 = NULL;
                            node_t *nps2 = NULL;
                            int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                syntax_error(program, item2, "already defined, previous in (%lld:%lld)",
                                    node->position.line, node->position.column);
                                return -1;
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_PROPERTY)
                {
                    node_property_t *property2 = (node_property_t *)item2->value;

                    if (syntax_idcmp(property1->key, property2->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = NULL;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *nps1 = NULL;
                            node_t *nps2 = NULL;
                            int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                syntax_error(program, item2, "already defined, previous in (%lld:%lld)",
                                    node->position.line, node->position.column);
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
        }
    }
    
    list_t *response1 = list_create();
    if (response1 == NULL)
    {
        fprintf(stderr, "%s-(%u):unable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    int32_t r1 = syntax_postfix(program, property1->type, response1, node);
    if (r1 == -1)
    {
        return -1;
    }
    else
    if (r1 == 0)
    {
        syntax_error(program, property1->type, "reference not found");
        return -1;
    }

    list_destroy(response1);

    return 1;
}

static int32_t
syntax_heritage(program_t *program, node_t *node)
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
                    if (syntax_idcmp(heritage1->key, heritage3->key) == 1)
                    {
                        syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                            item2->position.line, item2->position.column);
                        return -1;
                    }
                }
            }
            
            sub = current;
            current = current->parent;
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
                    if (item3->id == node->id)
                    {
                        break;
                    }

                    if (item3->kind == NODE_KIND_GENERIC)
                    {
                        node_generic_t *generic3 = (node_generic_t *)item3->value;
                        if (syntax_idcmp(heritage1->key, generic3->key) == 1)
                        {
                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                item3->position.line, item3->position.column);
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
                    if (syntax_idcmp(heritage1->key, class3->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = class3->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
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
                                if (item3->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *func3 = (node_func_t *)item3->value;
                                    if (syntax_idstrcmp(func3->key, "Constructor") == 1)
                                    {
                                        node_t *nps1 = NULL;
                                        node_t *nps2 = func3->parameters;
                                        int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            syntax_error(program, item2, "already defined, previous in (%lld:%lld)",
                                                node->position.line, node->position.column);
                                            return -1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_FUNC)
                {
                    node_func_t *func2 = (node_func_t *)item2->value;

                    if (syntax_idcmp(heritage1->key, func2->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = func2->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *nps1 = NULL;
                            node_t *nps2 = func2->parameters;
                            int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                syntax_error(program, item2, "already defined, previous in (%lld:%lld)",
                                    node->position.line, node->position.column);
                                return -1;
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_ENUM)
                {
                    node_enum_t *enum2 = (node_enum_t *)item2->value;

                    if (syntax_idcmp(heritage1->key, enum2->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = NULL;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *nps1 = NULL;
                            node_t *nps2 = NULL;
                            int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                syntax_error(program, item2, "already defined, previous in (%lld:%lld)",
                                    node->position.line, node->position.column);
                                return -1;
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_PROPERTY)
                {
                    node_property_t *property2 = (node_property_t *)item2->value;

                    if (syntax_idcmp(heritage1->key, property2->key) == 1)
                    {
                        node_t *ngs1 = NULL;
                        node_t *ngs2 = NULL;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *nps1 = NULL;
                            node_t *nps2 = NULL;
                            int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                syntax_error(program, item2, "already defined, previous in (%lld:%lld)",
                                    node->position.line, node->position.column);
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
        }
    }

    return 1;
}

static int32_t
syntax_heritages(program_t *program, node_t *node)
{
	node_block_t *heritages = (node_block_t *)node->value;
    ilist_t *a1;
    for (a1 = heritages->list->begin;a1 != heritages->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        int32_t result;
        result = syntax_heritage(program, item1);
        if (result == -1)
        {
            return -1;
        }
    }
	return 1;
}

static int32_t
syntax_class(program_t *program, node_t *node)
{
	node_class_t *class1 = (node_class_t *)node->value;
    node_t *node1 = class1->block;
    node_block_t *block1 = (node_block_t *)node1->value;

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
                    node_t *fields1 = import1->fields;
                    node_block_t *block3 = (node_block_t *)fields1->value;
                    ilist_t *a3;
                    for (a3 = block3->list->begin;a3 != block3->list->end;a3 = a3->next)
                    {
                        node_t *item3 = (node_t *)a3->value;
                        if (item3->kind == NODE_KIND_FIELD)
                        {
                            node_pair_t *field1 = (node_pair_t *)item3->value;
                            node_t *key1 = field1->key;
                            if (key1->kind == NODE_KIND_ID)
                            {
                                if (syntax_idcmp(key1, class1->key) == 1)
                                {
                                    syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                        key1->position.line, key1->position.column);
                                    return -1;
                                }
                            }
                            else
                            if (key1->kind == NODE_KIND_PSEUDONYM)
                            {
                                node_carrier_t *carrier1 = (node_carrier_t *)key1->value;
                                node_t *key2 = carrier1->base;
                                if (key2->kind == NODE_KIND_ID)
                                {
                                    if (syntax_idcmp(key2, class1->key) == 1)
                                    {
                                        node_t *ngs1 = class1->generics;
                                        node_t *nfs2 = carrier1->data;
                                        int32_t r1 = syntax_eqaul_gsfs(program, ngs1, nfs2);
                                        if (r1 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r1 == 1)
                                        {
                                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                                key1->position.line, key1->position.column);
                                            return -1;
                                        }
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
                    if (syntax_idcmp(class1->key, class3->key) == 1)
                    {
                        node_t *ngs1 = class1->generics;
                        node_t *ngs2 = class3->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            ilist_t *b1;
                            for (b1 = block1->list->begin;b1 != block1->list->end;b1 = b1->next)
                            {
                                node_t *item1 = (node_t *)b1->value;
                                if (item1->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *func1 = (node_func_t *)item1->value;
                                    if (syntax_idstrcmp(func1->key, "Constructor") == 1)
                                    {
                                        node_t *node3 = class3->block;
                                        node_block_t *block3 = (node_block_t *)node3->value;
                                        ilist_t *b3;
                                        for (b3 = block3->list->begin;b3 != block3->list->end;b3 = b3->next)
                                        {
                                            node_t *item3 = (node_t *)b3->value;
                                            if (item3->kind == NODE_KIND_FUNC)
                                            {
                                                node_func_t *func3 = (node_func_t *)item3->value;
                                                if (syntax_idstrcmp(func3->key, "Constructor") == 1)
                                                {
                                                    node_t *nps1 = func1->parameters;
                                                    node_t *nps2 = func3->parameters;
                                                    int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                                    if (r2 == -1)
                                                    {
                                                        return -1;
                                                    }
                                                    else
                                                    if (r2 == 1)
                                                    {
                                                        syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                                            item2->position.line, item2->position.column);
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

                    if (syntax_idcmp(class1->key, enum2->key) == 1)
                    {
                        node_t *ngs1 = class1->generics;
                        node_t *ngs2 = NULL;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            ilist_t *b1;
                            for (b1 = block1->list->begin;b1 != block1->list->end;b1 = b1->next)
                            {
                                node_t *item1 = (node_t *)b1->value;
                                if (item1->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *func1 = (node_func_t *)item1->value;
                                    if (syntax_idstrcmp(func1->key, "Constructor") == 1)
                                    {
                                        node_t *nps1 = func1->parameters;
                                        node_t *nps2 = NULL;
                                        int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                                item2->position.line, item2->position.column);
                                            return -1;
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
        if (current->kind == NODE_KIND_CLASS)
        {
            node_class_t *class2 = (node_class_t *)current->value;
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
                    if (syntax_idcmp(class1->key, class3->key) == 1)
                    {
                        node_t *ngs1 = class1->generics;
                        node_t *ngs2 = class3->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            ilist_t *b1;
                            for (b1 = block1->list->begin;b1 != block1->list->end;b1 = b1->next)
                            {
                                node_t *item1 = (node_t *)b1->value;
                                if (item1->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *func1 = (node_func_t *)item1->value;
                                    if (syntax_idstrcmp(func1->key, "Constructor") == 1)
                                    {
                                        node_t *node3 = class3->block;
                                        node_block_t *block3 = (node_block_t *)node3->value;
                                        ilist_t *b3;
                                        for (b3 = block3->list->begin;b3 != block3->list->end;b3 = b3->next)
                                        {
                                            node_t *item3 = (node_t *)b3->value;
                                            if (item3->kind == NODE_KIND_FUNC)
                                            {
                                                node_func_t *func3 = (node_func_t *)item3->value;
                                                if (syntax_idstrcmp(func3->key, "Constructor") == 1)
                                                {
                                                    node_t *nps1 = func1->parameters;
                                                    node_t *nps2 = func3->parameters;
                                                    int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                                    if (r2 == -1)
                                                    {
                                                        return -1;
                                                    }
                                                    else
                                                    if (r2 == 1)
                                                    {
                                                        syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                                            item2->position.line, item2->position.column);
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
                if (item2->kind == NODE_KIND_FUNC)
                {
                    node_func_t *func2 = (node_func_t *)item2->value;

                    if (syntax_idcmp(class1->key, func2->key) == 1)
                    {
                        node_t *ngs1 = class1->generics;
                        node_t *ngs2 = func2->generics;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            ilist_t *b1;
                            for (b1 = block1->list->begin;b1 != block1->list->end;b1 = b1->next)
                            {
                                node_t *item1 = (node_t *)b1->value;
                                if (item1->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *func1 = (node_func_t *)item1->value;
                                    if (syntax_idstrcmp(func1->key, "Constructor") == 1)
                                    {
                                        node_t *nps1 = func1->parameters;
                                        node_t *nps2 = func2->parameters;
                                        int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                                item2->position.line, item2->position.column);
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

                    if (syntax_idcmp(class1->key, enum2->key) == 1)
                    {
                        node_t *ngs1 = class1->generics;
                        node_t *ngs2 = NULL;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            ilist_t *b1;
                            for (b1 = block1->list->begin;b1 != block1->list->end;b1 = b1->next)
                            {
                                node_t *item1 = (node_t *)b1->value;
                                if (item1->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *func1 = (node_func_t *)item1->value;
                                    if (syntax_idstrcmp(func1->key, "Constructor") == 1)
                                    {
                                        node_t *nps1 = func1->parameters;
                                        node_t *nps2 = NULL;
                                        int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                                item2->position.line, item2->position.column);
                                            return -1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                else
                if (item2->kind == NODE_KIND_PROPERTY)
                {
                    node_property_t *property2 = (node_property_t *)item2->value;

                    if (syntax_idcmp(class1->key, property2->key) == 1)
                    {
                        node_t *ngs1 = class1->generics;
                        node_t *ngs2 = NULL;
                        int32_t r1 = syntax_eqaul_gsgs(program, ngs1, ngs2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            ilist_t *b1;
                            for (b1 = block1->list->begin;b1 != block1->list->end;b1 = b1->next)
                            {
                                node_t *item1 = (node_t *)b1->value;
                                if (item1->kind == NODE_KIND_FUNC)
                                {
                                    node_func_t *func1 = (node_func_t *)item1->value;
                                    if (syntax_idstrcmp(func1->key, "Constructor") == 1)
                                    {
                                        node_t *nps1 = func1->parameters;
                                        node_t *nps2 = NULL;
                                        int32_t r2 = syntax_eqaul_psps(program, nps1, nps2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            syntax_error(program, node, "already defined, previous in (%lld:%lld)",
                                                item2->position.line, item2->position.column);
                                            return -1;
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
        {
            current = current->parent;
        }
    }

    if (class1->generics != NULL)
    {
        int32_t r1 = syntax_generics(program, class1->generics);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (class1->heritages != NULL)
    {
        int32_t r1 = syntax_heritages(program, class1->heritages);
        if (r1 == -1)
        {
            return -1;
        }
    }

    ilist_t *a1;
    for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
    {
        node_t *item = (node_t *)a1->value;
        if (item->kind == NODE_KIND_CLASS)
        {
            int32_t result;
            result = syntax_class(program, item);
            if (result == -1)
            {
                return -1;
            }
        }
        else
        if (item->kind == NODE_KIND_ENUM)
        {
            int32_t result;
            result = syntax_enum(program, item);
            if (result == -1)
            {
                return -1;
            }
        }
        else
        if (item->kind == NODE_KIND_FUNC)
        {
            int32_t result;
            result = syntax_func(program, item);
            if (result == -1)
            {
                return -1;
            }
        }
        else
        if (item->kind == NODE_KIND_PROPERTY)
        {
            int32_t result;
            result = syntax_property(program, item);
            if (result == -1)
            {
                return -1;
            }
        }
    }

	return 1;
}

static int32_t
syntax_module(program_t *program, node_t *node)
{
	node_module_t *module = (node_module_t *)node->value;

    ilist_t *a1;
    for (a1 = module->items->begin; a1 != module->items->end; a1 = a1->next)
    {
        node_t *item = (node_t *)a1->value;
        if (item->kind == NODE_KIND_IMPORT)
        {
            int32_t result;
            result = syntax_import(program, item);
            if (result == -1)
            {
                return -1;
            }
        }
        else
        if (item->kind == NODE_KIND_CLASS)
        {
            int32_t result;
            result = syntax_class(program, item);
            if (result == -1)
            {
                return -1;
            }
        }
        else
        if (item->kind == NODE_KIND_ENUM)
        {
            int32_t result;
            result = syntax_enum(program, item);
            if (result == -1)
            {
                return -1;
            }
        }
    }

	return 1;
}

int32_t
syntax_run(program_t *program, node_t *node)
{
	int32_t result = syntax_module(program, node);
	if(result == -1)
	{
		return -1;
	}
	return 1;
}