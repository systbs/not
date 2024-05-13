#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#include "../../types/types.h"
#include "../../container/list.h"
#include "../../container/stack.h"
#include "../../token/position.h"
#include "../../token/token.h"
#include "../../program.h"
#include "../../scanner/scanner.h"
#include "../../ast/node.h"
#include "../../utils/utils.h"
#include "../../utils/path.h"
#include "../syntax/syntax.h"
#include "../error.h"
#include "semantic.h"

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
                                semantic_error(program, field1->key, "Naming:field key must be an identifier");
                                return -1;
                            }

                            node_field_t *field3 = (node_field_t *)item2->value;
                            if (field3->value != NULL)
                            {
                                if (field3->key->kind != NODE_KIND_ID)
                                {
                                    semantic_error(program, field3->key, "Naming:field key must be an identifier");
                                    return -1;
                                }
                                if (semantic_idcmp(field1->key, field3->key) == 1)
                                {
                                    node_t *key1 = field1->key;
                                    node_basic_t *key_string1 = key1->value;
                
                                    semantic_error(program, field1->key, "Naming:'%s' already defined, previous in (%lld:%lld)\n\tInternal:%s-%u",
                                        key_string1->value, field3->key->position.line, field3->key->position.column, __FILE__, __LINE__);
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
        int32_t r1 = semantic_field(program, item1, flag);
        if (r1 == -1)
        {
            return -1;
        }
    }
	return 1;
}

int32_t
semantic_pseudonym(program_t *program, node_t *base, node_t *node, list_t *response, uint64_t flag)
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
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    int32_t r1 = semantic_postfix(program, base, carrier->base, response1, flag);
    if (r1 == -1)
    {
        return -1;
    }
    else
    {
        ilist_t *a1;
        for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
        {
            node_t *item1 = (node_t *)a1->value;
            if (item1->kind == NODE_KIND_CLASS)
            {
                node_class_t *class1 = (node_class_t *)item1->value;
                int32_t r1 = semantic_eqaul_gsfs(program, class1->generics, carrier->data);
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
                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                }
                continue;
            }
            else
            if (item1->kind == NODE_KIND_PACKAGE)
            {
                node_package_t *package1 = (node_package_t *)item1->value;
                int32_t r1 = semantic_eqaul_gsfs(program, package1->generics, carrier->data);
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
                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                }
                continue;
            }
            else
            if (item1->kind == NODE_KIND_FUN)
            {
                node_fun_t *fun1 = (node_fun_t *)item1->value;
                int32_t r1 = semantic_eqaul_gsfs(program, fun1->generics, carrier->data);
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
                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                }
                continue;
            }
            else
            if (item1->kind == NODE_KIND_LAMBDA)
            {
                node_lambda_t *fun1 = (node_lambda_t *)item1->value;
                int32_t r1 = semantic_eqaul_gsfs(program, fun1->generics, carrier->data);
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
                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                }
                continue;
            }
            else
            if (item1->kind == NODE_KIND_VAR)
            {
                semantic_error(program, carrier->base, "Typing:content cannot be accessed\n\tInternal:%s-%u", 
                    __FILE__, __LINE__);
                return -1;
            }
            else
            if (item1->kind == NODE_KIND_ENTITY)
            {
                semantic_error(program, carrier->base, "Typing:content cannot be accessed\n\tInternal:%s-%u", 
                    __FILE__, __LINE__);
                return -1;
            }
            else
            if (item1->kind == NODE_KIND_PROPERTY)
            {
                semantic_error(program, carrier->base, "Typing:content cannot be accessed\n\tInternal:%s-%u", 
                    __FILE__, __LINE__);
                return -1;
            }
            else
            if (item1->kind == NODE_KIND_PARAMETER)
            {
                semantic_error(program, carrier->base, "Typing:content cannot be accessed\n\tInternal:%s-%u", 
                    __FILE__, __LINE__);
                return -1;
            }
            else
            if (item1->kind == NODE_KIND_HERITAGE)
            {
                semantic_error(program, carrier->base, "Typing:content cannot be accessed\n\tInternal:%s-%u", 
                    __FILE__, __LINE__);
                return -1;
            }
            else
            if (item1->kind == NODE_KIND_GENERIC)
            {
                semantic_error(program, carrier->base, "Typing:non-generic type\n\tInternal:%s-%u", 
                    __FILE__, __LINE__);
                return -1;
            }
            else
            {
                semantic_error(program, carrier->base, "Typing:non-generic type\n\tInternal:%s-%u", 
                    __FILE__, __LINE__);
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
