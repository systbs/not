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

int32_t
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

                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
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

int32_t
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

                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
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
semantic_vresolve(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    if (node->kind == NODE_KIND_VAR)
    {
        node_var_t *var1 = (node_var_t *)node->value;
        if (var1->value_update != NULL)
        {
            ilist_t *il1 = list_rpush(response, var1->value_update);
            if (il1 == NULL)
            {
                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                return -1;
            }
            return 1;
        }
        else
        {
            if (var1->value != NULL)
            {
                list_t *response1 = list_create();
                if (response1 == NULL)
                {
                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                    return -1;
                }

                int32_t r1 = semantic_expression(program, var1->value, response1, flag);
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
                            if ((item1->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                            {
                                semantic_error(program, node, "The value not an instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                return -1;
                            }

                            var1->value_update = item1;

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
                        if (item1->kind == NODE_KIND_LAMBDA)
                        {
                            var1->value_update = item1;

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
                        if (item1->kind == NODE_KIND_VAR)
                        {
                            int32_t r2 = semantic_vresolve(program, item1, response, flag);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            list_destroy(response1);
                            return 1;
                        }
                        else
                        if (item1->kind == NODE_KIND_ENTITY)
                        {
                            int32_t r2 = semantic_vresolve(program, item1, response, flag);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            list_destroy(response1);
                            return 1;
                        }
                        else
                        if (item1->kind == NODE_KIND_PROPERTY)
                        {
                            int32_t r2 = semantic_vresolve(program, item1, response, flag);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            list_destroy(response1);
                            return 1;
                        }
                        else
                        if (item1->kind == NODE_KIND_PARAMETER)
                        {
                            int32_t r2 = semantic_vresolve(program, item1, response, flag);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            list_destroy(response1);
                            return 1;
                        }
                        else
                        {
                            semantic_error(program, node, "The value not an instance of object or fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    if (cnt_response1 == 0)
                    {
                        semantic_error(program, var1->key, "The value has no valid result\n\tInternal:%s-%u", __FILE__, __LINE__);
                        return -1;
                    }
                }

                list_destroy(response1);
            }
            else
            {
                semantic_error(program, var1->key, "Unitialized\n\tInternal:%s-%u", __FILE__, __LINE__);
                return -1;
            }
        }
    }
    else
    if (node->kind == NODE_KIND_ENTITY)
    {
        node_entity_t *entity1 = (node_entity_t *)node->value;
        if (entity1->value_update != NULL)
        {
            ilist_t *il1 = list_rpush(response, entity1->value_update);
            if (il1 == NULL)
            {
                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                return -1;
            }
            return 1;
        }
        else
        {
            if (entity1->value != NULL)
            {
                list_t *response1 = list_create();
                if (response1 == NULL)
                {
                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                    return -1;
                }

                int32_t r1 = semantic_expression(program, entity1->value, response1, flag);
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
                            if ((item1->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                            {
                                semantic_error(program, node, "The value not an instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                return -1;
                            }

                            entity1->value_update = item1;

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
                        if (item1->kind == NODE_KIND_LAMBDA)
                        {
                            entity1->value_update = item1;

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
                        if (item1->kind == NODE_KIND_VAR)
                        {
                            int32_t r2 = semantic_vresolve(program, item1, response, flag);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            list_destroy(response1);
                            return 1;
                        }
                        else
                        if (item1->kind == NODE_KIND_ENTITY)
                        {
                            int32_t r2 = semantic_vresolve(program, item1, response, flag);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            list_destroy(response1);
                            return 1;
                        }
                        else
                        if (item1->kind == NODE_KIND_PROPERTY)
                        {
                            int32_t r2 = semantic_vresolve(program, item1, response, flag);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            list_destroy(response1);
                            return 1;
                        }
                        else
                        if (item1->kind == NODE_KIND_PARAMETER)
                        {
                            int32_t r2 = semantic_vresolve(program, item1, response, flag);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            list_destroy(response1);
                            return 1;
                        }
                        else
                        {
                            semantic_error(program, node, "The value not an instance of object or fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    if (cnt_response1 == 0)
                    {
                        semantic_error(program, entity1->key, "The value has no valid result\n\tInternal:%s-%u", __FILE__, __LINE__);
                        return -1;
                    }
                }

                list_destroy(response1);
            }
            else
            {
                semantic_error(program, entity1->key, "Unitialized\n\tInternal:%s-%u", __FILE__, __LINE__);
                return -1;
            }
        }
    }
    else
    if (node->kind == NODE_KIND_PROPERTY)
    {
        node_property_t *property1 = (node_property_t *)node->value;
        if (property1->value_update != NULL)
        {
            ilist_t *il1 = list_rpush(response, property1->value_update);
            if (il1 == NULL)
            {
                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                return -1;
            }
            return 1;
        }
        else
        {
            if (property1->value != NULL)
            {
                list_t *response1 = list_create();
                if (response1 == NULL)
                {
                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                    return -1;
                }

                int32_t r1 = semantic_expression(program, property1->value, response1, flag);
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
                            if ((item1->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                            {
                                semantic_error(program, node, "The value not an instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                return -1;
                            }

                            property1->value_update = item1;

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
                        if (item1->kind == NODE_KIND_LAMBDA)
                        {
                            property1->value_update = item1;

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
                        if (item1->kind == NODE_KIND_VAR)
                        {
                            int32_t r2 = semantic_vresolve(program, item1, response, flag);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            list_destroy(response1);
                            return 1;
                        }
                        else
                        if (item1->kind == NODE_KIND_ENTITY)
                        {
                            int32_t r2 = semantic_vresolve(program, item1, response, flag);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            list_destroy(response1);
                            return 1;
                        }
                        else
                        if (item1->kind == NODE_KIND_PROPERTY)
                        {
                            int32_t r2 = semantic_vresolve(program, item1, response, flag);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            list_destroy(response1);
                            return 1;
                        }
                        else
                        if (item1->kind == NODE_KIND_PARAMETER)
                        {
                            int32_t r2 = semantic_vresolve(program, item1, response, flag);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            list_destroy(response1);
                            return 1;
                        }
                        else
                        {
                            semantic_error(program, node, "The value not an instance of object or fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    if (cnt_response1 == 0)
                    {
                        semantic_error(program, property1->key, "The value has no valid result\n\tInternal:%s-%u", __FILE__, __LINE__);
                        return -1;
                    }
                }

                list_destroy(response1);
            }
            else
            {
                if (property1->type != NULL)
                {
                    node_t *type1 = property1->type;

                    if (type1->kind == NODE_KIND_FN)
                    {
                        node_fn_t *fun1 = (node_fn_t *)type1->value;

                        node_t *clone1 = node_create(type1->parent, type1->position);
                        if (clone1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }
                        clone1 = node_make_lambda(clone1, fun1->generics, fun1->parameters, NULL, fun1->result);

                        property1->value_update = clone1;

                        ilist_t *il1 = list_rpush(response, clone1);
                        if (il1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }
                        return 1;
                    }
                    else
                    {
                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }

                        int32_t r1 = semantic_resolve(program, type1->parent, type1, response1, flag);
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
                                    node_t *clone1 = node_clone(item1->parent, item1);
                                    if (clone1 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    clone1->flag |= NODE_FLAG_INSTANCE;

                                    property1->value_update = clone1;

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
                                    node_generic_t *generic1 = (node_generic_t *)item1->value;
                                    if (generic1->type != NULL)
                                    {
                                        list_t *response2 = list_create();
                                        if (response2 == NULL)
                                        {
                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
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

                                                    property1->value_update = clone1;

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
                                                semantic_error(program, generic1->type, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }

                                        list_destroy(response2);
                                    }
                                }
                                else
                                {
                                    semantic_error(program, type1, "Wrong type\n\tInternal:%s-%u", __FILE__, __LINE__);
                                    return -1;
                                }
                            }

                            if (cnt_response1 == 0)
                            {
                                semantic_error(program, type1, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                return -1;
                            }
                        }

                        list_destroy(response1);
                    }
                }
                else
                {
                    semantic_error(program, property1->key, "Unitialized\n\tInternal:%s-%u", __FILE__, __LINE__);
                    return -1;
                }
            }
        }
    }
    else
    if (node->kind == NODE_KIND_PARAMETER)
    {
        node_parameter_t *parameter1 = (node_parameter_t *)node->value;
        if (parameter1->value_update != NULL)
        {
            ilist_t *il1 = list_rpush(response, parameter1->value_update);
            if (il1 == NULL)
            {
                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                return -1;
            }
            return 1;
        }
        else
        {
            if (parameter1->value != NULL)
            {
                list_t *response1 = list_create();
                if (response1 == NULL)
                {
                    fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                    return -1;
                }

                int32_t r1 = semantic_expression(program, parameter1->value, response1, flag);
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
                            if ((item1->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                            {
                                semantic_error(program, node, "The value not an instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                return -1;
                            }

                            parameter1->value_update = item1;

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
                        if (item1->kind == NODE_KIND_LAMBDA)
                        {
                            parameter1->value_update = item1;

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
                        if (item1->kind == NODE_KIND_VAR)
                        {
                            int32_t r2 = semantic_vresolve(program, item1, response, flag);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            list_destroy(response1);
                            return 1;
                        }
                        else
                        if (item1->kind == NODE_KIND_ENTITY)
                        {
                            int32_t r2 = semantic_vresolve(program, item1, response, flag);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            list_destroy(response1);
                            return 1;
                        }
                        else
                        if (item1->kind == NODE_KIND_PROPERTY)
                        {
                            int32_t r2 = semantic_vresolve(program, item1, response, flag);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            list_destroy(response1);
                            return 1;
                        }
                        else
                        if (item1->kind == NODE_KIND_PARAMETER)
                        {
                            int32_t r2 = semantic_vresolve(program, item1, response, flag);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            list_destroy(response1);
                            return 1;
                        }
                        else
                        {
                            semantic_error(program, node, "The value not an instance of object or fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    if (cnt_response1 == 0)
                    {
                        semantic_error(program, parameter1->key, "The value has no valid result\n\tInternal:%s-%u", __FILE__, __LINE__);
                        return -1;
                    }
                }

                list_destroy(response1);
            }
            else
            {
                if (parameter1->type != NULL)
                {
                    node_t *type1 = parameter1->type;

                    if (type1->kind == NODE_KIND_FN)
                    {
                        node_fn_t *fun1 = (node_fn_t *)type1->value;

                        node_t *clone1 = node_create(type1->parent, type1->position);
                        if (clone1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }
                        clone1 = node_make_lambda(clone1, fun1->generics, fun1->parameters, NULL, fun1->result);

                        parameter1->value_update = clone1;

                        ilist_t *il1 = list_rpush(response, clone1);
                        if (il1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }
                        return 1;
                    }
                    else
                    {
                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }

                        int32_t r1 = semantic_resolve(program, type1->parent, type1, response1, flag);
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
                                    node_t *clone1 = node_clone(item1->parent, item1);
                                    if (clone1 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    clone1->flag |= NODE_FLAG_INSTANCE;

                                    parameter1->value_update = clone1;

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
                                    node_generic_t *generic1 = (node_generic_t *)item1->value;
                                    if (generic1->type != NULL)
                                    {
                                        list_t *response2 = list_create();
                                        if (response2 == NULL)
                                        {
                                            fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
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

                                                    parameter1->value_update = clone1;

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
                                                semantic_error(program, generic1->type, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }

                                        list_destroy(response2);
                                    }
                                }
                                else
                                {
                                    semantic_error(program, type1, "Wrong type\n\tInternal:%s-%u", __FILE__, __LINE__);
                                    return -1;
                                }
                            }

                            if (cnt_response1 == 0)
                            {
                                semantic_error(program, type1, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                return -1;
                            }
                        }

                        list_destroy(response1);
                    }
                }
                else
                {
                    semantic_error(program, parameter1->key, "Unitialized\n\tInternal:%s-%u", __FILE__, __LINE__);
                    return -1;
                }
            }
        }
    }
    else
    if (node->kind == NODE_KIND_CLASS)
    {
        if ((node->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
        {
            semantic_error(program, node, "Not an instance of object\n\tInternal:%s-%u", __FILE__, __LINE__);
            return -1;
        }

        ilist_t *il1 = list_rpush(response, node);
        if (il1 == NULL)
        {
            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }
        return 1;
    }

    semantic_error(program, node, "No content as instance of object\n\tInternal:%s-%u", __FILE__, __LINE__);
    return -1;
}
