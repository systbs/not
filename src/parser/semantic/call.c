#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#include "../../types/types.h"
#include "../../container/list.h"
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
semantic_gresolve(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    node_generic_t *generic1 = (node_generic_t *)node->value;
    if (generic1->type != NULL)
    {
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_resolve(program, generic1->type->parent, generic1->type, response1, flag);
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

                    if ((item1->kind & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                    {
                        semantic_error(program, generic1->key, "Instance object, in confronting with (%s-%lld:%lld)",
                            class1->key->position.path, class1->key->position.line, class1->key->position.column);
                        return -1;
                    }

                    ilist_t *il1 = list_rpush(response, item1);
                    if (il1 == NULL)
                    {
                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                    list_destroy(response1);
                    return 1;
                }
                else
                if (item1->kind == NODE_KIND_GENERIC)
                {
                    list_destroy(response1);
                    return semantic_gresolve(program, item1, response, flag);
                }
                else
                {
                    semantic_error(program, generic1->key, "Not valid\n\tInternal:%s-%u", __FILE__, __LINE__);
                    return -1;
                }
            }

            if (cnt_response1 == 0)
            {
                semantic_error(program, generic1->type, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                return -1;
            }
        }

        list_destroy(response1);
    }

    return 0;
}

static int32_t
semantic_hresolve(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    node_heritage_t *heritage1 = (node_heritage_t *)node->value;
    if (heritage1->type != NULL)
    {
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_resolve(program, heritage1->type->parent, heritage1->type, response1, flag);
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

                    if ((item1->kind & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                    {
                        semantic_error(program, heritage1->key, "Instance object, in confronting with (%s-%lld:%lld)",
                            class1->key->position.path, class1->key->position.line, class1->key->position.column);
                        return -1;
                    }

                    node_t *clone1 = node_clone(item1->parent, item1);
                    if (clone1 == NULL)
                    {
                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                    clone1->flag |= NODE_FLAG_INSTANCE;

                    heritage1->value_update = clone1;

                    ilist_t *il1 = list_rpush(response, clone1);
                    if (il1 == NULL)
                    {
                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                    list_destroy(response1);
                    return 1;
                }
                else
                if (item1->kind == NODE_KIND_GENERIC)
                {
                    list_t *response2 = list_create();
                    if (response2 == NULL)
                    {
                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r2 = semantic_gresolve(program, item1, response2, flag);
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
                                node_t *clone1 = node_clone(item2->parent, item2);
                                if (clone1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                clone1->flag |= NODE_FLAG_INSTANCE;

                                heritage1->value_update = clone1;

                                ilist_t *il1 = list_rpush(response, clone1);
                                if (il1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                list_destroy(response2);
                                list_destroy(response1);
                                return 1;
                            }
                        }

                        if (cnt_response2 == 0)
                        {
                            semantic_error(program, item1, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    list_destroy(response2);
                }
                else
                {
                    semantic_error(program, heritage1->key, "Not valid\n\tInternal:%s-%u", __FILE__, __LINE__);
                    return -1;
                }
            }

            if (cnt_response1 == 0)
            {
                semantic_error(program, heritage1->type, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                return -1;
            }
        }

        list_destroy(response1);
    }

    return 0;
}

int32_t
semantic_call(program_t *program, node_t *base, node_t *node, list_t *response, uint64_t flag)
{
    node_carrier_t *carrier = (node_carrier_t *)node->value;

    list_t *response1 = list_create();
    if (response1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    int32_t r1 = semantic_resolve(program, base, carrier->base, response1, flag);
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

                node_t *node1 = class1->block;
                node_block_t *block1 = (node_block_t *)node1->value;

                ilist_t *a2;
                for (a2 = block1->list->begin;a2 != block1->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    if (item2->kind == NODE_KIND_FUN)
                    {
                        node_fun_t *fun1 = (node_fun_t *)item2->value;
                        if (semantic_idstrcmp(fun1->key, "Constructor") == 1)
                        {
                            int32_t r1 = semantic_eqaul_psas(program, fun1->parameters, carrier->data);
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
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                clone1->flag |= NODE_FLAG_INSTANCE;

                                ilist_t *il1 = list_rpush(response, clone1);
                                if (il1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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
                int32_t r1 = semantic_eqaul_psas(program, fun1->parameters, carrier->data);
                if (r1 == -1)
                {
                    return -1;
                }
                else
                if (r1 == 1)
                {
                    node_t *node1 = fun1->result;

                    list_t *response2 = list_create();
                    if (response2 == NULL)
                    {
                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r2 = semantic_resolve(program, node1->parent, node1, response2, flag);
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
                                node_t *clone1 = node_clone(item2->parent, item2);
                                if (clone1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                clone1->flag |= NODE_FLAG_INSTANCE;

                                ilist_t *il1 = list_rpush(response, clone1);
                                if (il1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                list_destroy(response2);
                                list_destroy(response1);
                                return 1;
                            }
                        }

                        if (cnt_response2 == 0)
                        {
                            semantic_error(program, node1, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    list_destroy(response2);
                }
            }
            else
            if (item1->kind == NODE_KIND_LAMBDA)
            {
                node_lambda_t *fun1 = (node_lambda_t *)item1->value;
                int32_t r1 = semantic_eqaul_psas(program, fun1->parameters, carrier->data);
                if (r1 == -1)
                {
                    return -1;
                }
                else
                if (r1 == 1)
                {
                    node_t *node1 = fun1->result;

                    list_t *response2 = list_create();
                    if (response2 == NULL)
                    {
                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r2 = semantic_resolve(program, node1->parent, node1, response2, flag);
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
                                node_t *clone1 = node_clone(item2->parent, item2);
                                if (clone1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                clone1->flag |= NODE_FLAG_INSTANCE;

                                ilist_t *il1 = list_rpush(response, clone1);
                                if (il1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                list_destroy(response2);
                                list_destroy(response1);
                                return 1;
                            }
                        }

                        if (cnt_response2 == 0)
                        {
                            semantic_error(program, node1, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    list_destroy(response2);
                }
            }
            else
            if (item1->kind == NODE_KIND_VAR)
            {
                node_var_t *var1 = (node_var_t *)item1->value;
                if (var1->type != NULL)
                {
                    node_t *node1 = var1->type;
                    if (node1->kind == NODE_KIND_FN)
                    {
                        node_fn_t *fn1 = (node_fn_t *)node1->value;
                        int32_t r1 = semantic_eqaul_psas(program, fn1->parameters, carrier->data);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *node2 = fn1->result;

                            list_t *response2 = list_create();
                            if (response2 == NULL)
                            {
                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r2 = semantic_resolve(program, node2->parent, node2, response2, flag);
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
                                        node_t *clone1 = node_clone(item2->parent, item2);
                                        if (clone1 == NULL)
                                        {
                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }
                                        clone1->flag |= NODE_FLAG_INSTANCE;

                                        ilist_t *il1 = list_rpush(response, clone1);
                                        if (il1 == NULL)
                                        {
                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }
                                        list_destroy(response2);
                                        list_destroy(response1);
                                        return 1;
                                    }
                                }

                                if (cnt_response2 == 0)
                                {
                                    semantic_error(program, node2, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                    return -1;
                                }
                            }

                            list_destroy(response2);
                        }
                    }
                    continue;
                }
                else
                {
                    node_t *node1 = var1->value_update;
                    if (node1 != NULL)
                    {
                        if (node1->kind == NODE_KIND_LAMBDA)
                        {
                            node_lambda_t *fun1 = (node_lambda_t *)node1->value;
                            int32_t r1 = semantic_eqaul_psas(program, fun1->parameters, carrier->data);
                            if (r1 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r1 == 1)
                            {
                                node_t *node2 = fun1->result;

                                list_t *response2 = list_create();
                                if (response2 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }

                                int32_t r2 = semantic_resolve(program, node2->parent, node2, response2, flag);
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
                                            node_t *clone1 = node_clone(item2->parent, item2);
                                            if (clone1 == NULL)
                                            {
                                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            clone1->flag |= NODE_FLAG_INSTANCE;

                                            ilist_t *il1 = list_rpush(response, clone1);
                                            if (il1 == NULL)
                                            {
                                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }
                                    }

                                    if (cnt_response2 == 0)
                                    {
                                        semantic_error(program, node2, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                        return -1;
                                    }
                                }

                                list_destroy(response2);
                            }
                        }
                    }
                    else
                    {
                        semantic_error(program, carrier->base, "Unitialized\n\tInternal:%s-%u", __FILE__, __LINE__);
                        return -1;
                    }
                    continue;
                }
            }
            else
            if (item1->kind == NODE_KIND_ENTITY)
            {
                node_entity_t *entity1 = (node_entity_t *)item1->value;
                if (entity1->type != NULL)
                {
                    node_t *node1 = entity1->type;
                    if (node1->kind == NODE_KIND_FN)
                    {
                        node_fn_t *fn1 = (node_fn_t *)node1->value;
                        int32_t r1 = semantic_eqaul_psas(program, fn1->parameters, carrier->data);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *node2 = fn1->result;

                            list_t *response2 = list_create();
                            if (response2 == NULL)
                            {
                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r2 = semantic_resolve(program, node2->parent, node2, response2, flag);
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
                                        node_t *clone1 = node_clone(item2->parent, item2);
                                        if (clone1 == NULL)
                                        {
                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }
                                        clone1->flag |= NODE_FLAG_INSTANCE;

                                        ilist_t *il1 = list_rpush(response, clone1);
                                        if (il1 == NULL)
                                        {
                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }
                                        list_destroy(response2);
                                        list_destroy(response1);
                                        return 1;
                                    }
                                }

                                if (cnt_response2 == 0)
                                {
                                    semantic_error(program, node2, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                    return -1;
                                }
                            }

                            list_destroy(response2);
                        }
                    }
                    continue;
                }
                else
                {
                    node_t *node1 = entity1->value_update;
                    if (node1 != NULL)
                    {
                        if (node1->kind == NODE_KIND_LAMBDA)
                        {
                            node_lambda_t *fun1 = (node_lambda_t *)node1->value;
                            int32_t r1 = semantic_eqaul_psas(program, fun1->parameters, carrier->data);
                            if (r1 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r1 == 1)
                            {
                                node_t *node2 = fun1->result;

                                list_t *response2 = list_create();
                                if (response2 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }

                                int32_t r2 = semantic_resolve(program, node2->parent, node2, response2, flag);
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
                                            node_t *clone1 = node_clone(item2->parent, item2);
                                            if (clone1 == NULL)
                                            {
                                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            clone1->flag |= NODE_FLAG_INSTANCE;

                                            ilist_t *il1 = list_rpush(response, clone1);
                                            if (il1 == NULL)
                                            {
                                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }
                                    }

                                    if (cnt_response2 == 0)
                                    {
                                        semantic_error(program, node2, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                        return -1;
                                    }
                                }

                                list_destroy(response2);
                            }
                        }
                    }
                    else
                    {
                        semantic_error(program, carrier->base, "Unitialized\n\tInternal:%s-%u", __FILE__, __LINE__);
                        return -1;
                    }
                    continue;
                }
            }
            else
            if (item1->kind == NODE_KIND_PROPERTY)
            {
                node_property_t *property1 = (node_property_t *)item1->value;
                if (property1->type != NULL)
                {
                    node_t *node1 = property1->type;
                    if (node1->kind == NODE_KIND_FN)
                    {
                        node_fn_t *fn1 = (node_fn_t *)node1->value;
                        int32_t r1 = semantic_eqaul_psas(program, fn1->parameters, carrier->data);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *node2 = fn1->result;

                            list_t *response2 = list_create();
                            if (response2 == NULL)
                            {
                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r2 = semantic_resolve(program, node2->parent, node2, response2, flag);
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
                                        node_t *clone1 = node_clone(item2->parent, item2);
                                        if (clone1 == NULL)
                                        {
                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }
                                        clone1->flag |= NODE_FLAG_INSTANCE;

                                        ilist_t *il1 = list_rpush(response, clone1);
                                        if (il1 == NULL)
                                        {
                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }
                                        list_destroy(response2);
                                        list_destroy(response1);
                                        return 1;
                                    }
                                }

                                if (cnt_response2 == 0)
                                {
                                    semantic_error(program, node2, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                    return -1;
                                }
                            }

                            list_destroy(response2);
                        }
                    }
                    continue;
                }
                else
                {
                    node_t *node1 = property1->value_update;
                    if (node1 != NULL)
                    {
                        if (node1->kind == NODE_KIND_LAMBDA)
                        {
                            node_lambda_t *fun1 = (node_lambda_t *)node1->value;
                            int32_t r1 = semantic_eqaul_psas(program, fun1->parameters, carrier->data);
                            if (r1 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r1 == 1)
                            {
                                node_t *node2 = fun1->result;

                                list_t *response2 = list_create();
                                if (response2 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }

                                int32_t r2 = semantic_resolve(program, node2->parent, node2, response2, flag);
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
                                            node_t *clone1 = node_clone(item2->parent, item2);
                                            if (clone1 == NULL)
                                            {
                                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            clone1->flag |= NODE_FLAG_INSTANCE;

                                            ilist_t *il1 = list_rpush(response, clone1);
                                            if (il1 == NULL)
                                            {
                                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }
                                    }

                                    if (cnt_response2 == 0)
                                    {
                                        semantic_error(program, node2, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                        return -1;
                                    }
                                }

                                list_destroy(response2);
                            }
                        }
                    }
                    else
                    {
                        list_t *response2 = list_create();
                        if (response2 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }

                        int32_t r2 = semantic_expression(program, property1->value, response2, flag);
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
                                if (item2->kind == NODE_KIND_LAMBDA)
                                {
                                    node_lambda_t *fun1 = (node_lambda_t *)item2->value;
                                    int32_t r3 = semantic_eqaul_psas(program, fun1->parameters, carrier->data);
                                    if (r3 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r3 == 1)
                                    {
                                        property1->value_update = item2;
                                        node_t *node2 = fun1->result;

                                        list_t *response3 = list_create();
                                        if (response3 == NULL)
                                        {
                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }

                                        int32_t r4 = semantic_resolve(program, node2->parent, node2, response3, flag);
                                        if (r4 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        {
                                            uint64_t cnt_response3 = 0;

                                            ilist_t *a3;
                                            for (a3 = response3->begin;a3 != response3->end;a3 = a3->next)
                                            {
                                                cnt_response3 += 1;

                                                node_t *item3 = (node_t *)a3->value;

                                                if (item3->kind == NODE_KIND_CLASS)
                                                {
                                                    node_t *clone1 = node_clone(item3->parent, item3);
                                                    if (clone1 == NULL)
                                                    {
                                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                    clone1->flag |= NODE_FLAG_INSTANCE;

                                                    ilist_t *il1 = list_rpush(response, clone1);
                                                    if (il1 == NULL)
                                                    {
                                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                    list_destroy(response3);
                                                    list_destroy(response2);
                                                    list_destroy(response1);
                                                    return 1;
                                                }
                                            }

                                            if (cnt_response3 == 0)
                                            {
                                                semantic_error(program, node2, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }

                                        list_destroy(response3);
                                    }
                                }
                                else
                                {
                                    semantic_error(program, carrier->base, "Non-Generic\n\tInternal:%s-%u", __FILE__, __LINE__);
                                    return -1;
                                }
                            }

                            if (cnt_response2 == 0)
                            {
                                semantic_error(program, property1->value, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                return -1;
                            }
                        }

                        list_destroy(response2);
                    }
                    continue;
                }
            }
            else
            if (item1->kind == NODE_KIND_PARAMETER)
            {
                node_parameter_t *parameter1 = (node_parameter_t *)item1->value;
                if (parameter1->type != NULL)
                {
                    node_t *node1 = parameter1->type;
                    if (node1->kind == NODE_KIND_FN)
                    {
                        node_fn_t *fn1 = (node_fn_t *)node1->value;
                        int32_t r1 = semantic_eqaul_psas(program, fn1->parameters, carrier->data);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            node_t *node2 = fn1->result;

                            list_t *response2 = list_create();
                            if (response2 == NULL)
                            {
                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r2 = semantic_resolve(program, node2->parent, node2, response2, flag);
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
                                        node_t *clone1 = node_clone(item2->parent, item2);
                                        if (clone1 == NULL)
                                        {
                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }
                                        clone1->flag |= NODE_FLAG_INSTANCE;

                                        ilist_t *il1 = list_rpush(response, clone1);
                                        if (il1 == NULL)
                                        {
                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }
                                        list_destroy(response2);
                                        list_destroy(response1);
                                        return 1;
                                    }
                                }

                                if (cnt_response2 == 0)
                                {
                                    semantic_error(program, node2, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                    return -1;
                                }
                            }

                            list_destroy(response2);
                        }
                    }
                    continue;
                }
                else
                {
                    semantic_error(program, carrier->base, "Unitialized\n\tInternal:%s-%u", __FILE__, __LINE__);
                    return -1;
                }
            }
            else
            if (item1->kind == NODE_KIND_GENERIC)
            {
                node_t *node1 = item1;
                while (node1 != NULL)
                {
                    node_generic_t *generic1 = (node_generic_t *)node1->value;
                    if (generic1->type != NULL)
                    {
                        list_t *response2 = list_create();
                        if (response2 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }

                        int32_t r2 = semantic_resolve(program, generic1->type->parent, generic1->type, response2, flag);
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
                                    node_class_t *class1 = (node_class_t *)item2->value;

                                    node_t *node3 = class1->block;
                                    node_block_t *block1 = (node_block_t *)node3->value;

                                    ilist_t *a3;
                                    for (a3 = block1->list->begin;a3 != block1->list->end;a3 = a3->next)
                                    {
                                        node_t *item3 = (node_t *)a3->value;
                                        if (item3->kind == NODE_KIND_FUN)
                                        {
                                            node_fun_t *fun1 = (node_fun_t *)item3->value;
                                            if (semantic_idstrcmp(fun1->key, "Constructor") == 1)
                                            {
                                                int32_t r4 = semantic_eqaul_psas(program, fun1->parameters, carrier->data);
                                                if (r4 == -1)
                                                {
                                                    return -1;
                                                }
                                                else
                                                if (r4 == 1)
                                                {
                                                    node_t *clone1 = node_clone(item2->parent, item2);
                                                    if (clone1 == NULL)
                                                    {
                                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                    clone1->flag |= NODE_FLAG_INSTANCE;

                                                    ilist_t *il1 = list_rpush(response, clone1);
                                                    if (il1 == NULL)
                                                    {
                                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                    list_destroy(response2);
                                                    list_destroy(response1);
                                                    return 1;
                                                }
                                            }
                                        }
                                    }

                                    node1 = NULL;
                                }
                                else
                                if (item2->kind == NODE_KIND_GENERIC)
                                {
                                    node1 = item2;
                                    break;
                                }
                            }

                            if (cnt_response2 == 0)
                            {
                                semantic_error(program, generic1->type, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                return -1;
                            }
                        }
                        list_destroy(response2);
                    }
                    else
                    {
                        node1 = NULL;
                        break;
                    }
                }
            }
            else
            if (item1->kind == NODE_KIND_HERITAGE)
            {
                node_heritage_t *heritage1 = (node_heritage_t *)item1->value;
                if (heritage1->value_update != NULL)
                {
                    node_t *node1 = heritage1->value_update;
                    if (node1->kind == NODE_KIND_CLASS)
                    {
                        node_class_t *class1 = (node_class_t *)node1->value;

                        node_t *node2 = class1->block;
                        node_block_t *block1 = (node_block_t *)node2->value;

                        ilist_t *a2;
                        for (a2 = block1->list->begin;a2 != block1->list->end;a2 = a2->next)
                        {
                            node_t *item2 = (node_t *)a2->value;
                            if (item2->kind == NODE_KIND_FUN)
                            {
                                node_fun_t *fun1 = (node_fun_t *)item2->value;
                                if (semantic_idstrcmp(fun1->key, "Constructor") == 1)
                                {
                                    int32_t r1 = semantic_eqaul_psas(program, fun1->parameters, carrier->data);
                                    if (r1 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r1 == 1)
                                    {
                                        node_t *clone1 = node_clone(node1->parent, node1);
                                        if (clone1 == NULL)
                                        {
                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }
                                        clone1->flag |= NODE_FLAG_INSTANCE;

                                        ilist_t *il1 = list_rpush(response, clone1);
                                        if (il1 == NULL)
                                        {
                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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
                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r2 = semantic_hresolve(program, item1, response2, flag);
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
                                node_class_t *class1 = (node_class_t *)item2->value;

                                node_t *node3 = class1->block;
                                node_block_t *block1 = (node_block_t *)node3->value;

                                ilist_t *a3;
                                for (a3 = block1->list->begin;a3 != block1->list->end;a3 = a3->next)
                                {
                                    node_t *item3 = (node_t *)a3->value;
                                    if (item3->kind == NODE_KIND_FUN)
                                    {
                                        node_fun_t *fun1 = (node_fun_t *)item3->value;
                                        if (semantic_idstrcmp(fun1->key, "Constructor") == 1)
                                        {
                                            int32_t r3 = semantic_eqaul_psas(program, fun1->parameters, carrier->data);
                                            if (r3 == -1)
                                            {
                                                return -1;
                                            }
                                            else
                                            if (r3 == 1)
                                            {
                                                ilist_t *il1 = list_rpush(response, item2);
                                                if (il1 == NULL)
                                                {
                                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                list_destroy(response2);
                                                list_destroy(response1);
                                                return 1;
                                            }
                                        }
                                    }
                                }

                            }
                        }

                        if (cnt_response2 == 0)
                        {
                            semantic_error(program, heritage1->key, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    list_destroy(response2);
                }
            }
        }

        if (cnt_response1 == 0)
        {
            semantic_error(program, carrier->base, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
            return -1;
        }

        semantic_error(program, carrier->base, "Wrong call\n\tInternal:%s-%u", __FILE__, __LINE__);
        return -1;
    }

    list_destroy(response1);

    return 0;
}
