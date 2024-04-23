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
semantic_postfix(program_t *program, node_t *base, node_t *node, list_t *response, uint64_t flag)
{
	if (node->kind == NODE_KIND_CALL)
    {
        return semantic_call(program, base, node, response, flag);
    }
    else
    if (node->kind == NODE_KIND_ATTRIBUTE)
    {
        return semantic_attribute(program, base, node, response, flag);
    }
    else
    if (node->kind == NODE_KIND_PSEUDONYM)
    {
        return semantic_pseudonym(program, base, node, response, flag);
    }
    else
    {
        return semantic_primary(program, base, node, response, flag);
    }
}

int32_t
semantic_resolve(program_t *program, node_t *base, node_t *node, list_t *response, uint64_t flag)
{
    list_t *response1 = list_create();
    if (response1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    int32_t r1 = semantic_postfix(program, node->parent, node, response1, flag);
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
                    semantic_error(program, root1, "Using not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                    return -1;
                }

                node_using_t *using1 = (node_using_t *)root1->value;

                node_basic_t *basic2 = (node_basic_t *)using1->path->value;

                node_t *module1 = program_load(program, basic2->value);
                if (module1 == NULL)
                {
                    return -1;
                }

                list_t *response2 = list_create();
                if (response2 == NULL)
                {
                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                    return -1;
                }

                int32_t r2 = semantic_postfix(program, module1, package1->address, response2, SEMANTIC_FLAG_NONE);
                if (r2 == -1)
                {
                    return -1;
                }
                else
                if (r2 == 1)
                {
                    uint64_t cnt_response2 = 0;

                    ilist_t *b1;
                    for (b1 = response2->begin;b1 != response2->end;b1 = b1->next)
                    {
                        cnt_response2 += 1;

                        node_t *item2 = (node_t *)b1->value;
                        if (item2->kind == NODE_KIND_CLASS)
                        {
                            ilist_t *il1 = list_rpush(response, item2);
                            if (il1 == NULL)
                            {
                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }

                    if (cnt_response2 == 0)
                    {
                        semantic_error(program, package1->address, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                        return -1;
                    }
                }
                list_destroy(response2);
            }
            else
            {
                ilist_t *il1 = list_rpush(response, item1);
                if (il1 == NULL)
                {
                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                    return -1;
                }
            }
        }
        
        if (cnt_response1 == 0)
        {
            semantic_error(program, node, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
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
semantic_content(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    list_t *response1 = list_create();
    if (response1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    int32_t r1 = semantic_resolve(program, node->parent, node, response1, flag);
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

            if (item1->kind == NODE_KIND_HERITAGE)
            {
                node_heritage_t *heritage1 = (node_heritage_t *)item1->value;
                if (heritage1->value_update != NULL)
                {
                    node_t *node1 = heritage1->value_update;
                    ilist_t *il1 = list_rpush(response, node1);
                    if (il1 == NULL)
                    {
                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                    list_destroy(response1);
                    return 1;
                }
                else
                {
                    list_t *response2 = list_create();
                    if (response2 == NULL)
                    {
                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r2 = semantic_resolve(program, heritage1->type->parent, heritage1->type, response2, flag);
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
                            else
                            if (item2->kind == NODE_KIND_GENERIC)
                            {
                                node_t *node1 = item2;

                                while (node1 != NULL)
                                {
                                    node_generic_t *generic1 = (node_generic_t *)node1->value;
                                    if (generic1->type != NULL)
                                    {
                                        list_t *response3 = list_create();
                                        if (response3 == NULL)
                                        {
                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }

                                        int32_t r3 = semantic_resolve(program, generic1->type->parent, generic1->type, response3, flag);
                                        if (r3 == -1)
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

                                                    heritage1->value_update = clone1;

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
                                                else
                                                if (item3->kind == NODE_KIND_GENERIC)
                                                {
                                                    node1 = item3;
                                                    break;
                                                }
                                            }

                                            if (cnt_response3 == 0)
                                            {
                                                semantic_error(program, generic1->type, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }

                                        list_destroy(response3);
                                    }
                                    else
                                    {
                                        ilist_t *il1 = list_rpush(response, node1);
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

                        if (cnt_response2 == 0)
                        {
                            semantic_error(program, heritage1->type, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    list_destroy(response2);
                }
            }
            else
            {
                ilist_t *il1 = list_rpush(response, item1);
                if (il1 == NULL)
                {
                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                    return -1;
                }
                list_destroy(response1);
                return 1;
            }
        }

        if (cnt_response1 == 0)
        {
            semantic_error(program, node, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
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
semantic_cresolve(program_t *program, node_t *node, list_t *response, char *OPERATOR_SIMBOL, node_t *arguments, uint64_t flag)
{
    node_class_t *class1 = (node_class_t *)node->value;

    node_t *node1 = class1->block;
    node_block_t *block1 = node1->value;

    ilist_t *a2;
    for (a2 = block1->list->begin;a2 != block1->list->end;a2 = a2->next)
    {
        node_t *item2 = (node_t *)a2->value;
        if (item2->kind == NODE_KIND_FUN)
        {
            node_fun_t *fun1 = (node_fun_t *)item2->value;
            if (semantic_idstrcmp(fun1->key, OPERATOR_SIMBOL) == 1)
            {
                int32_t r2 = semantic_eqaul_psas(program, fun1->parameters, arguments);
                if (r2 == -1)
                {
                    return -1;
                }
                else
                if (r2 == 1)
                {
                    if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                    {
                        semantic_error(program, fun1->key, "Private access\n\tInternal:%s-%u", __FILE__, __LINE__);
                        return -1;
                    }

                    list_t *response2 = list_create();
                    if (response2 == NULL)
                    {
                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r3 = semantic_resolve(program, fun1->result->parent, fun1->result, response2, SEMANTIC_FLAG_NONE);
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
                                list_destroy(response2);
                                return 1;
                            }
                            else
                            if (item3->kind == NODE_KIND_GENERIC)
                            {
                                list_t *response3 = list_create();
                                if (response3 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }

                                int32_t r4 = semantic_gresolve(program, item3, response3, flag);
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

                                        node_t *item4 = (node_t *)a3->value;
                                        if (item4->kind == NODE_KIND_CLASS)
                                        {
                                            node_t *clone1 = node_clone(item4->parent, item4);
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
                                            return 1;
                                        }
                                    }

                                    if (cnt_response3 == 0)
                                    {
                                        semantic_error(program, item3, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                        return -1;
                                    }
                                }

                                list_destroy(response3);

                                ilist_t *il1 = list_rpush(response, item3);
                                if (il1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                list_destroy(response2);
                                return 1;
                            }
                        }

                        if (cnt_response2 == 0)
                        {
                            semantic_error(program, fun1->result, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                            return -1;
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
semantic_dispart(program_t *program, node_t *node, list_t *response, char *OPERATOR_SIMBOL, node_t *arguments, node_t *major, uint64_t flag)
{
    if (node->kind == NODE_KIND_CLASS)
    {
        node_class_t *class1 = (node_class_t *)node->value;

        if ((node->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
        {
            semantic_error(program, major, "Not an instance object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                class1->key->position.path, class1->key->position.line, class1->key->position.column ,__FILE__, __LINE__);
            return -1;
        }

        int32_t r2 = semantic_cresolve(program, node, response, OPERATOR_SIMBOL, arguments, flag);
        if (r2 == -1)
        {
            return -1;
        }
        else
        if (r2 == 1)
        {
            return 1;
        }
        
        semantic_error(program, major, "Lack of %s operator, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
            OPERATOR_SIMBOL, class1->key->position.path, class1->key->position.line, class1->key->position.column ,__FILE__, __LINE__);
        return -1;
    }
    else
    if (node->kind == NODE_KIND_VAR)
    {
        node_var_t *var1 = (node_var_t *)node->value;
        if (var1->value_update != NULL)
        {
            node_t *value_update1 = var1->value_update;

            if (value_update1->kind == NODE_KIND_CLASS)
            {
                node_class_t *class1 = (node_class_t *)value_update1->value;

                if ((value_update1->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                {
                    semantic_error(program, major, "Not an instance object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                        class1->key->position.path, class1->key->position.line, class1->key->position.column ,__FILE__, __LINE__);
                    return -1;
                }

                int32_t r2 = semantic_cresolve(program, value_update1, response, OPERATOR_SIMBOL, arguments, flag);
                if (r2 == -1)
                {
                    return -1;
                }
                else
                if (r2 == 1)
                {
                    return 1;
                }
                
                semantic_error(program, major, "Lack of %s operator, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                    OPERATOR_SIMBOL, class1->key->position.path, class1->key->position.line, class1->key->position.column ,__FILE__, __LINE__);
                return -1;
            }
            else
            {
                semantic_error(program, major, "Not an object\n\tInternal:%s-%u", __FILE__, __LINE__);
                return -1;
            }
        }
        else
        {
            semantic_error(program, major, "Unitialized\n\tInternal:%s-%u", __FILE__, __LINE__);
            return -1;
        }
    }
    else
    if (node->kind == NODE_KIND_ENTITY)
    {
        node_entity_t *entity1 = (node_entity_t *)node->value;
        if (entity1->value_update != NULL)
        {
            node_t *value_update1 = entity1->value_update;

            if (value_update1->kind == NODE_KIND_CLASS)
            {
                node_class_t *class1 = (node_class_t *)value_update1->value;

                if ((value_update1->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                {
                    semantic_error(program, major, "Not an instance object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                        class1->key->position.path, class1->key->position.line, class1->key->position.column ,__FILE__, __LINE__);
                    return -1;
                }

                int32_t r2 = semantic_cresolve(program, value_update1, response, OPERATOR_SIMBOL, arguments, flag);
                if (r2 == -1)
                {
                    return -1;
                }
                else
                if (r2 == 1)
                {
                    return 1;
                }

                semantic_error(program, major, "Lack of %s operator, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                    OPERATOR_SIMBOL, class1->key->position.path, class1->key->position.line, class1->key->position.column ,__FILE__, __LINE__);
                return -1;
            }
            else
            {
                semantic_error(program, major, "Not an object\n\tInternal:%s-%u", __FILE__, __LINE__);
                return -1;
            }
        }
        else
        {
            semantic_error(program, major, "Unitialized\n\tInternal:%s-%u", __FILE__, __LINE__);
            return -1;
        }
    }
    else
    if (node->kind == NODE_KIND_PROPERTY)
    {
        node_property_t *property1 = (node_property_t *)node->value;
        if (property1->value_update != NULL)
        {
            node_t *value_update1 = property1->value_update;

            if (value_update1->kind == NODE_KIND_CLASS)
            {
                node_class_t *class1 = (node_class_t *)value_update1->value;

                if ((value_update1->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                {
                    semantic_error(program, major, "Not an instance, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                        class1->key->position.path, class1->key->position.line, class1->key->position.column ,__FILE__, __LINE__);
                    return -1;
                }

                int32_t r2 = semantic_cresolve(program, value_update1, response, OPERATOR_SIMBOL, arguments, flag);
                if (r2 == -1)
                {
                    return -1;
                }
                else
                if (r2 == 1)
                {
                    return 1;
                }

                semantic_error(program, major, "Lack of %s operator, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                    OPERATOR_SIMBOL, class1->key->position.path, class1->key->position.line, class1->key->position.column ,__FILE__, __LINE__);
                return -1;
            }
            else
            {
                semantic_error(program, major, "Not an object\n\tInternal:%s-%u", __FILE__, __LINE__);
                return -1;
            }
        }
        else
        {
            if (property1->type != NULL)
            {
                node_t *node1 = property1->type;
                if (node1->kind == NODE_KIND_FN)
                {
                    semantic_error(program, major, "Not an object\n\tInternal:%s-%u", __FILE__, __LINE__);
                    return -1;
                }
                else
                {
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
                                node_class_t *class1 = (node_class_t *)item2->value;

                                if ((item2->kind & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                {
                                    semantic_error(program, node1, "Instance object, in confronting with (%s-%lld:%lld)",
                                        class1->key->position.path, class1->key->position.line, class1->key->position.column);
                                    return -1;
                                }

                                node_t *clone1 = node_clone(item2->parent, item2);
                                if (clone1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                clone1->flag |= NODE_FLAG_INSTANCE;

                                property1->value_update = clone1;

                                int32_t r3 = semantic_cresolve(program, clone1, response, OPERATOR_SIMBOL, arguments, flag);
                                if (r3 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r3 == 1)
                                {
                                    return 1;
                                }

                                node_class_t *class2 = (node_class_t *)clone1->value;
                                semantic_error(program, major, "Lack of %s operator, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    OPERATOR_SIMBOL, class2->key->position.path, class2->key->position.line, class2->key->position.column ,__FILE__, __LINE__);
                                return -1;
                            }
                            else
                            if (item2->kind == NODE_KIND_GENERIC)
                            {
                                node_generic_t *generic1 = (node_generic_t *)item2->value;

                                list_t *response3 = list_create();
                                if (response3 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }

                                int32_t r3 = semantic_gresolve(program, item2, response3, flag);
                                if (r3 == -1)
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

                                            property1->value_update = clone1;

                                            int32_t r4 = semantic_cresolve(program, clone1, response, OPERATOR_SIMBOL, arguments, flag);
                                            if (r4 == -1)
                                            {
                                                return -1;
                                            }
                                            else
                                            if (r4 == 1)
                                            {
                                                list_destroy(response3);
                                                list_destroy(response2);
                                                return 1;
                                            }

                                            node_class_t *class2 = (node_class_t *)clone1->value;
                                            semantic_error(program, major, "Lack of %s operator, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                OPERATOR_SIMBOL, class2->key->position.path, class2->key->position.line, class2->key->position.column ,__FILE__, __LINE__);
                                            return -1;
                                        }
                                    }

                                    if (cnt_response3 == 0)
                                    {
                                        semantic_error(program, item2, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                        return -1;
                                    }
                                }

                                list_destroy(response3);

                                semantic_error(program, major, "Lack of %s operator, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    OPERATOR_SIMBOL, generic1->key->position.path, generic1->key->position.line, generic1->key->position.column ,__FILE__, __LINE__);
                                return -1;
                            }
                            else
                            {
                                semantic_error(program, node1, "Wrong type\n\tInternal:%s-%u", __FILE__, __LINE__);
                                return -1;
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
            {
                node_t *node1 = property1->value;

                list_t *response2 = list_create();
                if (response2 == NULL)
                {
                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                    return -1;
                }

                int32_t r2 = semantic_expression(program, node1, response2, flag);
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

                            if ((item2->kind & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                            {
                                semantic_error(program, node1, "Instance object, in confronting with (%s-%lld:%lld)",
                                    class1->key->position.path, class1->key->position.line, class1->key->position.column);
                                return -1;
                            }

                            node_t *clone1 = node_clone(item2->parent, item2);
                            if (clone1 == NULL)
                            {
                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }
                            clone1->flag |= NODE_FLAG_INSTANCE;

                            property1->value_update = clone1;

                            int32_t r3 = semantic_cresolve(program, clone1, response, OPERATOR_SIMBOL, arguments, flag);
                            if (r3 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r3 == 1)
                            {
                                return 1;
                            }

                            node_class_t *class2 = (node_class_t *)clone1->value;
                            semantic_error(program, major, "Lack of %s operator, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                OPERATOR_SIMBOL, class2->key->position.path, class2->key->position.line, class2->key->position.column ,__FILE__, __LINE__);
                            return -1;
                        }
                        else
                        if (item2->kind == NODE_KIND_GENERIC)
                        {
                            node_generic_t *generic1 = (node_generic_t *)item2->value;

                            list_t *response3 = list_create();
                            if (response3 == NULL)
                            {
                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r3 = semantic_gresolve(program, item2, response3, flag);
                            if (r3 == -1)
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

                                        property1->value_update = clone1;

                                        int32_t r4 = semantic_cresolve(program, clone1, response, OPERATOR_SIMBOL, arguments, flag);
                                        if (r4 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r4 == 1)
                                        {
                                            list_destroy(response3);
                                            list_destroy(response2);
                                            return 1;
                                        }

                                        node_class_t *class2 = (node_class_t *)clone1->value;
                                        semantic_error(program, major, "Lack of %s operator, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            OPERATOR_SIMBOL, class2->key->position.path, class2->key->position.line, class2->key->position.column ,__FILE__, __LINE__);
                                        return -1;
                                    }
                                }

                                if (cnt_response3 == 0)
                                {
                                    semantic_error(program, item2, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                    return -1;
                                }
                            }

                            list_destroy(response3);

                            semantic_error(program, major, "Lack of %s operator, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                OPERATOR_SIMBOL, generic1->key->position.path, generic1->key->position.line, generic1->key->position.column ,__FILE__, __LINE__);
                            return -1;
                        }
                        else
                        {
                            semantic_error(program, node1, "Wrong type\n\tInternal:%s-%u", __FILE__, __LINE__);
                            return -1;
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
    }
    else
    if (node->kind == NODE_KIND_PARAMETER)
    {
        node_parameter_t *parameter1 = (node_parameter_t *)node->value;
        if (parameter1->value_update != NULL)
        {
            node_t *value_update1 = parameter1->value_update;

            if (value_update1->kind == NODE_KIND_CLASS)
            {
                node_class_t *class1 = (node_class_t *)value_update1->value;

                if ((value_update1->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                {
                    semantic_error(program, major, "Not an instance, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                        class1->key->position.path, class1->key->position.line, class1->key->position.column ,__FILE__, __LINE__);
                    return -1;
                }

                int32_t r2 = semantic_cresolve(program, value_update1, response, OPERATOR_SIMBOL, arguments, flag);
                if (r2 == -1)
                {
                    return -1;
                }
                else
                if (r2 == 1)
                {
                    return 1;
                }

                semantic_error(program, major, "Lack of %s operator, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                    OPERATOR_SIMBOL, class1->key->position.path, class1->key->position.line, class1->key->position.column ,__FILE__, __LINE__);
                return -1;
            }
            else
            {
                semantic_error(program, major, "Not an object\n\tInternal:%s-%u", __FILE__, __LINE__);
                return -1;
            }
        }
        else
        {
            if (parameter1->type != NULL)
            {
                node_t *node1 = parameter1->type;
                if (node1->kind == NODE_KIND_FN)
                {
                    semantic_error(program, major, "Not an object\n\tInternal:%s-%u", __FILE__, __LINE__);
                    return -1;
                }
                else
                {
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
                                node_class_t *class1 = (node_class_t *)item2->value;

                                if ((item2->kind & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                {
                                    semantic_error(program, node1, "Instance object, in confronting with (%s-%lld:%lld)",
                                        class1->key->position.path, class1->key->position.line, class1->key->position.column);
                                    return -1;
                                }

                                node_t *clone1 = node_clone(item2->parent, item2);
                                if (clone1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                clone1->flag |= NODE_FLAG_INSTANCE;

                                parameter1->value_update = clone1;

                                int32_t r3 = semantic_cresolve(program, clone1, response, OPERATOR_SIMBOL, arguments, flag);
                                if (r3 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r3 == 1)
                                {
                                    return 1;
                                }

                                node_class_t *class2 = (node_class_t *)clone1->value;
                                semantic_error(program, major, "Lack of %s operator, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    OPERATOR_SIMBOL, class2->key->position.path, class2->key->position.line, class2->key->position.column ,__FILE__, __LINE__);
                                return -1;
                            }
                            else
                            if (item2->kind == NODE_KIND_GENERIC)
                            {
                                node_generic_t *generic1 = (node_generic_t *)item2->value;

                                list_t *response3 = list_create();
                                if (response3 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }

                                int32_t r3 = semantic_gresolve(program, item2, response3, flag);
                                if (r3 == -1)
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

                                            parameter1->value_update = clone1;

                                            int32_t r4 = semantic_cresolve(program, clone1, response, OPERATOR_SIMBOL, arguments, flag);
                                            if (r4 == -1)
                                            {
                                                return -1;
                                            }
                                            else
                                            if (r4 == 1)
                                            {
                                                list_destroy(response3);
                                                list_destroy(response2);
                                                return 1;
                                            }

                                            node_class_t *class2 = (node_class_t *)clone1->value;
                                            semantic_error(program, major, "Lack of %s operator, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                OPERATOR_SIMBOL, class2->key->position.path, class2->key->position.line, class2->key->position.column ,__FILE__, __LINE__);
                                            return -1;
                                        }
                                    }

                                    if (cnt_response3 == 0)
                                    {
                                        semantic_error(program, item2, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                        return -1;
                                    }
                                }

                                list_destroy(response3);

                                semantic_error(program, major, "Lack of %s operator, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    OPERATOR_SIMBOL, generic1->key->position.path, generic1->key->position.line, generic1->key->position.column ,__FILE__, __LINE__);
                                return -1;
                            }
                            else
                            {
                                semantic_error(program, node1, "Wrong type\n\tInternal:%s-%u", __FILE__, __LINE__);
                                return -1;
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
            {
                node_t *node1 = parameter1->value;

                list_t *response2 = list_create();
                if (response2 == NULL)
                {
                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                    return -1;
                }

                int32_t r2 = semantic_expression(program, node1, response2, flag);
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

                            if ((item2->kind & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                            {
                                semantic_error(program, node1, "Instance object, in confronting with (%s-%lld:%lld)",
                                    class1->key->position.path, class1->key->position.line, class1->key->position.column);
                                return -1;
                            }

                            node_t *clone1 = node_clone(item2->parent, item2);
                            if (clone1 == NULL)
                            {
                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }
                            clone1->flag |= NODE_FLAG_INSTANCE;

                            parameter1->value_update = clone1;

                            int32_t r3 = semantic_cresolve(program, clone1, response, OPERATOR_SIMBOL, arguments, flag);
                            if (r3 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r3 == 1)
                            {
                                return 1;
                            }

                            node_class_t *class2 = (node_class_t *)clone1->value;
                            semantic_error(program, major, "Lack of %s operator, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                OPERATOR_SIMBOL, class2->key->position.path, class2->key->position.line, class2->key->position.column ,__FILE__, __LINE__);
                            return -1;
                        }
                        else
                        if (item2->kind == NODE_KIND_GENERIC)
                        {
                            node_generic_t *generic1 = (node_generic_t *)item2->value;

                            list_t *response3 = list_create();
                            if (response3 == NULL)
                            {
                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r3 = semantic_gresolve(program, item2, response3, flag);
                            if (r3 == -1)
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

                                        parameter1->value_update = clone1;

                                        int32_t r4 = semantic_cresolve(program, clone1, response, OPERATOR_SIMBOL, arguments, flag);
                                        if (r4 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r4 == 1)
                                        {
                                            list_destroy(response3);
                                            list_destroy(response2);
                                            return 1;
                                        }

                                        node_class_t *class2 = (node_class_t *)clone1->value;
                                        semantic_error(program, major, "Lack of %s operator, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            OPERATOR_SIMBOL, class2->key->position.path, class2->key->position.line, class2->key->position.column ,__FILE__, __LINE__);
                                        return -1;
                                    }
                                }

                                if (cnt_response3 == 0)
                                {
                                    semantic_error(program, item2, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                    return -1;
                                }
                            }

                            list_destroy(response3);

                            semantic_error(program, major, "Lack of %s operator, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                OPERATOR_SIMBOL, generic1->key->position.path, generic1->key->position.line, generic1->key->position.column ,__FILE__, __LINE__);
                            return -1;
                        }
                        else
                        {
                            semantic_error(program, node1, "Wrong type\n\tInternal:%s-%u", __FILE__, __LINE__);
                            return -1;
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
    }
    else
    {
        semantic_error(program, major, "Not an instance object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
            node->position.path, node->position.line, node->position.column ,__FILE__, __LINE__);
        return -1;
    }
    return 0;
}

static int32_t
semantic_fsolve(program_t *program, node_t *node, list_t *response, char *OPERATOR_SIMBOL, node_t *arguments, int32_t (*f)(program_t *, node_t *, list_t *, uint64_t), uint64_t flag)
{
    list_t *response1 = list_create();
    if (response1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    int32_t r1 = f(program, node, response1, SEMANTIC_FLAG_NONE);
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
            
            int32_t r2 = semantic_dispart(program, item1, response, OPERATOR_SIMBOL, arguments, node, flag);
            if (r2 == -1)
            {
                return -1;
            }
            else
            if (r2 == 0)
            {
                semantic_error(program, node, "Not an object\n\tInternal:%s-%u" ,__FILE__, __LINE__);
                return -1;
            }

            list_destroy(response1);
            return 1;
        }

        if (cnt_response1 == 0)
        {
            semantic_error(program, node, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
            return -1;
        }
    }

    list_destroy(response1);
    return 0;
}

static int32_t
semantic_unary(program_t *program, node_t *node, list_t *response, char *OPERATOR_SIMBOL, int32_t (*f)(program_t *, node_t *, list_t *, uint64_t), uint64_t flag)
{
    node_unary_t *unary1 = (node_unary_t *)node->value;

    return semantic_fsolve(program, unary1->right, response, OPERATOR_SIMBOL, NULL, f, flag);
}

static int32_t
semantic_binary(program_t *program, node_t *node, list_t *response, char *OPERATOR_SIMBOL, int32_t (*f)(program_t *, node_t *, list_t *, uint64_t), uint64_t flag)
{
    node_binary_t *binary1 = (node_binary_t *)node->value;

    node_t *arguments1 = semantic_make_arguments(program, binary1->right, 1, binary1->right);
    if (arguments1 == NULL)
    {
        return -1;
    }

    return semantic_fsolve(program, binary1->left, response, OPERATOR_SIMBOL, arguments1, f, flag);
}

static int32_t
semantic_prefix(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    if (node->kind == NODE_KIND_TILDE)
    {
        return semantic_unary(program, node, response, "~", semantic_prefix, SEMANTIC_FLAG_NONE);
    }
    else
    if (node->kind == NODE_KIND_POS)
    {
        return semantic_unary(program, node, response, "+", semantic_prefix, SEMANTIC_FLAG_NONE);
    }
    else
    if (node->kind == NODE_KIND_NEG)
    {
        return semantic_unary(program, node, response, "-", semantic_prefix, SEMANTIC_FLAG_NONE);
    }
    else
    if (node->kind == NODE_KIND_NOT)
    {
        return semantic_unary(program, node, response, "!", semantic_prefix, SEMANTIC_FLAG_NONE);
    }
    else
    {
        return semantic_content(program, node, response, flag);
    }
}

static int32_t
semantic_pow(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    if (node->kind == NODE_KIND_POW)
    {
        return semantic_binary(program, node, response, "**", semantic_pow, SEMANTIC_FLAG_NONE);
    }
    else
    {
        return semantic_prefix(program, node, response, flag);
    }
}

static int32_t
semantic_multipicative(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    if (node->kind == NODE_KIND_MUL)
    {
        return semantic_binary(program, node, response, "*", semantic_multipicative, SEMANTIC_FLAG_NONE);
    }
    else
    if (node->kind == NODE_KIND_DIV)
    {
        return semantic_binary(program, node, response, "/", semantic_multipicative, SEMANTIC_FLAG_NONE);
    }
    else
    if (node->kind == NODE_KIND_MOD)
    {
        return semantic_binary(program, node, response, "%", semantic_multipicative, SEMANTIC_FLAG_NONE);
    }
    else
    if (node->kind == NODE_KIND_EPI)
    {
        return semantic_binary(program, node, response, "\\", semantic_multipicative, SEMANTIC_FLAG_NONE);
    }
    else
    {
        return semantic_pow(program, node, response, flag);
    }
}

static int32_t
semantic_addative(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    if (node->kind == NODE_KIND_PLUS)
    {
        return semantic_binary(program, node, response, "+", semantic_addative, SEMANTIC_FLAG_NONE);
    }
    else
    if (node->kind == NODE_KIND_MINUS)
    {
        return semantic_binary(program, node, response, "-", semantic_addative, SEMANTIC_FLAG_NONE);
    }
    else
    {
        return semantic_multipicative(program, node, response, flag);
    }
}

static int32_t
semantic_shifting(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    if (node->kind == NODE_KIND_SHR)
    {
        return semantic_binary(program, node, response, ">>", semantic_addative, SEMANTIC_FLAG_NONE);
    }
    else
    if (node->kind == NODE_KIND_SHL)
    {
        return semantic_binary(program, node, response, "<<", semantic_addative, SEMANTIC_FLAG_NONE);
    }
    else
    {
        return semantic_addative(program, node, response, flag);
    }
}

static int32_t
semantic_relational(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    if (node->kind == NODE_KIND_LT)
    {
        return semantic_binary(program, node, response, "<", semantic_shifting, SEMANTIC_FLAG_NONE);
    }
    else
    if (node->kind == NODE_KIND_LE)
    {
        return semantic_binary(program, node, response, "<=", semantic_shifting, SEMANTIC_FLAG_NONE);
    }
    else
    if (node->kind == NODE_KIND_GT)
    {
        return semantic_binary(program, node, response, ">", semantic_shifting, SEMANTIC_FLAG_NONE);
    }
    else
    if (node->kind == NODE_KIND_GE)
    {
        return semantic_binary(program, node, response, ">=", semantic_shifting, SEMANTIC_FLAG_NONE);
    }
    else
    {
        return semantic_shifting(program, node, response, flag);
    }
}

static int32_t
semantic_equality(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    if (node->kind == NODE_KIND_EQ)
    {
        return semantic_binary(program, node, response, "==", semantic_relational, SEMANTIC_FLAG_NONE);
    }
    else
    if (node->kind == NODE_KIND_NEQ)
    {
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_binary(program, node, response1, "==", semantic_relational, SEMANTIC_FLAG_NONE);
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

                int32_t r2 = semantic_dispart(program, item1, response, "!", NULL, node, SEMANTIC_FLAG_NONE);
                if (r2 == -1)
                {
                    return -1;
                }
                else
                if (r2 == 0)
                {
                    semantic_error(program, node, "Not an object\n\tInternal:%s-%u" ,__FILE__, __LINE__);
                    return -1;
                }

                list_destroy(response1);
                return 1;
            }

            if (cnt_response1 == 0)
            {
                semantic_error(program, node, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                return -1;
            }
        }

        list_destroy(response1);
        return 0;
    }
    else
    {
        return semantic_relational(program, node, response, flag);
    }
}

static int32_t
semantic_bitwise_and(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    if (node->kind == NODE_KIND_AND)
    {
        return semantic_binary(program, node, response, "&", semantic_bitwise_and, SEMANTIC_FLAG_NONE);
    }
    else
    {
        return semantic_equality(program, node, response, flag);
    }
}

static int32_t
semantic_bitwise_xor(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    if (node->kind == NODE_KIND_XOR)
    {
        return semantic_binary(program, node, response, "^", semantic_bitwise_xor, SEMANTIC_FLAG_NONE);
    }
    else
    {
        return semantic_bitwise_and(program, node, response, flag);
    }
}

static int32_t
semantic_bitwise_or(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    if (node->kind == NODE_KIND_OR)
    {
        return semantic_binary(program, node, response, "|", semantic_bitwise_or, SEMANTIC_FLAG_NONE);
    }
    else
    {
        return semantic_bitwise_xor(program, node, response, flag);
    }
}

static int32_t
semantic_logical_and(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    if (node->kind == NODE_KIND_LAND)
    {
        return semantic_binary(program, node, response, "&&", semantic_logical_and, SEMANTIC_FLAG_NONE);
    }
    else
    {
        return semantic_bitwise_or(program, node, response, flag);
    }
}

static int32_t
semantic_logical_or(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    if (node->kind == NODE_KIND_LOR)
    {
        return semantic_binary(program, node, response, "||", semantic_logical_or, SEMANTIC_FLAG_NONE);
    }
    else
    {
        return semantic_logical_and(program, node, response, flag);
    }
}

int32_t
semantic_expression(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
	return semantic_logical_or(program, node, response, flag);
}

int32_t
semantic_assign(program_t *program, node_t *node, uint64_t flag)
{
    if ((node->kind == NODE_KIND_ASSIGN) ||
        (node->kind == NODE_KIND_ADD_ASSIGN) ||
        (node->kind == NODE_KIND_SUB_ASSIGN) ||
        (node->kind == NODE_KIND_MUL_ASSIGN) ||
        (node->kind == NODE_KIND_DIV_ASSIGN) ||
        (node->kind == NODE_KIND_EPI_ASSIGN) ||
        (node->kind == NODE_KIND_MOD_ASSIGN) ||
        (node->kind == NODE_KIND_AND_ASSIGN) ||
        (node->kind == NODE_KIND_OR_ASSIGN) ||
        (node->kind == NODE_KIND_SHL_ASSIGN) ||
        (node->kind == NODE_KIND_SHR_ASSIGN))
    {
        node_binary_t *binary1 = (node_binary_t *)node->value;

        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_expression(program, binary1->left, response1, SEMANTIC_FLAG_NONE);
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

                list_t *response2 = list_create();
                if (response2 == NULL)
                {
                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                    return -1;
                }

                int32_t r2 = 0;
                if (node->kind == NODE_KIND_ADD_ASSIGN)
                {
                    r2 = semantic_binary(program, node, response2, "+", semantic_expression, SEMANTIC_FLAG_NONE);
                }
                else
                if (node->kind == NODE_KIND_SUB_ASSIGN)
                {
                    r2 = semantic_binary(program, node, response2, "-", semantic_expression, SEMANTIC_FLAG_NONE);
                }
                else
                if (node->kind == NODE_KIND_MUL_ASSIGN)
                {
                    r2 = semantic_binary(program, node, response2, "*", semantic_expression, SEMANTIC_FLAG_NONE);
                }
                else
                if (node->kind == NODE_KIND_DIV_ASSIGN)
                {
                    r2 = semantic_binary(program, node, response2, "/", semantic_expression, SEMANTIC_FLAG_NONE);
                }
                else
                if (node->kind == NODE_KIND_EPI_ASSIGN)
                {
                    r2 = semantic_binary(program, node, response2, "\\", semantic_expression, SEMANTIC_FLAG_NONE);
                }
                else
                if (node->kind == NODE_KIND_MOD_ASSIGN)
                {
                    r2 = semantic_binary(program, node, response2, "%", semantic_expression, SEMANTIC_FLAG_NONE);
                }
                else
                if (node->kind == NODE_KIND_AND_ASSIGN)
                {
                    r2 = semantic_binary(program, node, response2, "&", semantic_expression, SEMANTIC_FLAG_NONE);
                }
                else
                if (node->kind == NODE_KIND_OR_ASSIGN)
                {
                    r2 = semantic_binary(program, node, response2, "|", semantic_expression, SEMANTIC_FLAG_NONE);
                }
                else
                if (node->kind == NODE_KIND_SHL_ASSIGN)
                {
                    r2 = semantic_binary(program, node, response2, "<<", semantic_expression, SEMANTIC_FLAG_NONE);
                }
                else
                if (node->kind == NODE_KIND_SHR_ASSIGN)
                {
                    r2 = semantic_binary(program, node, response2, ">>", semantic_expression, SEMANTIC_FLAG_NONE);
                }
                else
                {
                    r2 = semantic_expression(program, binary1->right, response2, SEMANTIC_FLAG_NONE);
                }

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

                        if (item1->kind == NODE_KIND_VAR)
                        {
                            if (item2->kind == NODE_KIND_VAR)
                            {

                            }
                            else
                            if (item2->kind == NODE_KIND_ENTITY)
                            {

                            }
                            else
                            if (item2->kind == NODE_KIND_PROPERTY)
                            {

                            }
                            else
                            if (item2->kind == NODE_KIND_PARAMETER)
                            {

                            }
                            else
                            if (item2->kind == NODE_KIND_CLASS)
                            {

                            }
                            else
                            if (item2->kind == NODE_KIND_LAMBDA)
                            {

                            }
                            else
                            {
                                semantic_error(program, node, "Not assignable\n\tInternal:%s-%u", __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }

                    if (cnt_response2 == 0)
                    {
                        semantic_error(program, binary1->right, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                        return -1;
                    }
                }

                list_destroy(response2);
            }

            if (cnt_response1 == 0)
            {
                semantic_error(program, binary1->left, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                return -1;
            }
        }

        list_destroy(response1);
        
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

        int32_t r1 = semantic_expression(program, node, response1, SEMANTIC_FLAG_NONE);
        if (r1 == -1)
        {
            return -1;
        }
        else
        if (r1 == 0)
        {
            semantic_error(program, node, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
            return -1;
        }

        list_destroy(response1);
        return 1;
    }
}