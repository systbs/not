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

                int32_t r2 = semantic_postfix(program, module1, package1->address, response2, flag);
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
        ilist_t *a1;
        for (a1 = response1->begin;a1 != response1->end;a1 = a1->next)
        {
            node_t *item1 = (node_t *)a1->value;

            if (item1->kind == NODE_KIND_HERITAGE)
            {
                node_heritage_t *heritage1 = (node_heritage_t *)item1->value;
                if (heritage1->value_update != NULL)
                {
                    node_t *value_update1 = heritage1->value_update;
                    ilist_t *il1 = list_rpush(response, value_update1);
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

                    int32_t r2 = semantic_expression(program, heritage1->type, response2, flag);
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
                                if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    node_t *key1 = heritage1->key;
                                    node_basic_t *key_string1 = key1->value;

                                    node_class_t *class1 = (node_class_t *)item2->value;

                                    node_t *key2 = class1->key;
                                    node_basic_t *key_string2 = key2->value;

                                    semantic_error(program, key1, "Typing:heritage '%s' has a instance type of class '%s'\n\tInternal:%s-%u",
                                        key_string1->value, key_string2->value, __FILE__, __LINE__);
                                    return -1;
                                }

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
                                if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    node_t *key1 = heritage1->key;
                                    node_basic_t *key_string1 = key1->value;

                                    node_generic_t *generic1 = (node_generic_t *)item2->value;

                                    node_t *key2 = generic1->key;
                                    node_basic_t *key_string2 = key2->value;

                                    semantic_error(program, key1, "Typing:heritage '%s' has a instance type of generic '%s'\n\tInternal:%s-%u",
                                        key_string1->value, key_string2->value, __FILE__, __LINE__);
                                    return -1;
                                }

                                node_generic_t *generic1 = (node_generic_t *)item2->value;
                                if (generic1->type != NULL)
                                {
                                    list_t *response3 = list_create();
                                    if (response3 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r3 = semantic_gresolve(program, generic1->type, response3, flag);
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
                                                if ((item3->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                {
                                                    node_t *key1 = generic1->key;
                                                    node_basic_t *key_string1 = key1->value;

                                                    node_class_t *class1 = (node_class_t *)item3->value;

                                                    node_t *key2 = class1->key;
                                                    node_basic_t *key_string2 = key2->value;

                                                    semantic_error(program, key1, "Typing:generic '%s' has a instance type of class '%s'\n\tInternal:%s-%u",
                                                        key_string1->value, key_string2->value, __FILE__, __LINE__);
                                                    return -1;
                                                }

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
                                                if ((item3->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                {
                                                    node_t *key1 = generic1->key;
                                                    node_basic_t *key_string1 = key1->value;

                                                    node_generic_t *generic2 = (node_generic_t *)item3->value;

                                                    node_t *key2 = generic2->key;
                                                    node_basic_t *key_string2 = key2->value;

                                                    semantic_error(program, key1, "Typing:generic '%s' has a instance type of generic '%s'\n\tInternal:%s-%u",
                                                        key_string1->value, key_string2->value, __FILE__, __LINE__);
                                                    return -1;
                                                }

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
    }

    list_destroy(response1);

    if (list_count(response) > 0)
    {
        return 1;
    }

    return 0;
}

static int32_t
semantic_export(program_t *program, node_t *node1, node_t *node2, node_t *major, char *OPERATOR_SIMBOL, list_t *response, uint64_t flag)
{
    if (node1->kind == NODE_KIND_CLASS)
    {
        node_class_t *class1 = (node_class_t *)node1->value;

        node_t *node3 = class1->block;
        node_block_t *block1 = node3->value;

        ilist_t *a2;
        for (a2 = block1->list->begin;a2 != block1->list->end;a2 = a2->next)
        {
            node_t *item2 = (node_t *)a2->value;
            if (item2->kind == NODE_KIND_FUN)
            {
                node_fun_t *fun1 = (node_fun_t *)item2->value;
                if (semantic_idstrcmp(fun1->key, OPERATOR_SIMBOL) == 1)
                {
                    int32_t r2 = semantic_eqaul_psns(program, fun1->parameters, node2);
                    if (r2 == -1)
                    {
                        return -1;
                    }
                    else
                    if (r2 == 1)
                    {
                        if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                        {
                            semantic_error(program, major, "Private access, in confronting with (%s-%lld-%lld)\n\tInternal:%s-%u",
                                fun1->key->position.path, fun1->key->position.line, fun1->key->position.column, __FILE__, __LINE__);
                            return -1;
                        }

                        list_t *response2 = list_create();
                        if (response2 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }

                        int32_t r3 = semantic_resolve(program, fun1->result->parent, fun1->result, response2, flag);
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
    else
    {
        semantic_error(program, major, "In rhs/lhs class is expected, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
            node1->position.path, node1->position.line, node1->position.column, __FILE__, __LINE__);
        return -1;
    }
}

static int32_t
semantic_dispart(program_t *program, node_t *node1, node_t *node2, node_t *major, char *OPERATOR_SIMBOL, list_t *response, uint64_t flag)
{
    list_t *response1 = list_create();
    if (response1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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

            if (node2 != NULL)
            {
                list_t *response2 = list_create();
                if (response2 == NULL)
                {
                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                    return -1;
                }

                int32_t r2 = semantic_vresolve(program, node2, response2, flag);
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

                        int32_t r3 = semantic_export(program, item1, item2, major, OPERATOR_SIMBOL, response, flag);
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
                        else
                        {
                            list_destroy(response2);
                            list_destroy(response1);
                            return 1;
                        }
                    }

                    if (cnt_response2 == 0)
                    {
                        semantic_error(program, major, "No value in extraction rhs/lhs, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                        return -1;
                    }
                }

                list_destroy(response2);
            }
            else
            {
                int32_t r3 = semantic_export(program, item1, NULL, major, OPERATOR_SIMBOL, response, flag);
                if (r3 == -1)
                {
                    return -1;
                }
                else
                if (r3 == 0)
                {
                    list_destroy(response1);
                    return 0;
                }
                else
                {
                    list_destroy(response1);
                    return 1;
                }
            }
        }

        if (cnt_response1 == 0)
        {
            semantic_error(program, major, "No value in extraction rhs/lhs, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                node1->position.path, node1->position.line, node1->position.column, __FILE__, __LINE__);
            return -1;
        }
    }

    list_destroy(response1);
    
    return 0;
}

static int32_t
semantic_fsolve(program_t *program, node_t *node1, node_t *node2, node_t *major, char *OPERATOR_SIMBOL, int32_t (*f)(program_t *, node_t *, list_t *, uint64_t), list_t *response, uint64_t flag)
{
    {
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = f(program, node1, response1, flag);
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
                
                if (node2 != NULL)
                {
                    list_t *response2 = list_create();
                    if (response2 == NULL)
                    {
                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r2 = semantic_content(program, node2, response2, flag);
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

                            int32_t r3 = semantic_dispart(program, item1, item2, major, OPERATOR_SIMBOL, response, flag);
                            if (r3 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r3 == 0)
                            {
                                list_destroy(response2);
                                list_destroy(response1);
                                goto region_step_2;
                            }
                            else
                            {
                                list_destroy(response2);
                                list_destroy(response1);
                                return 1;
                            }
                        }

                        if (cnt_response2 == 0)
                        {
                            semantic_error(program, major, "Rhs not result\n\tInternal:%s-%u", __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    list_destroy(response2);
                }
                else
                {
                    int32_t r2 = semantic_dispart(program, item1, NULL, major, OPERATOR_SIMBOL, response, flag);
                    if (r2 == -1)
                    {
                        return -1;
                    }
                    else
                    if (r2 == 0)
                    {
                        semantic_error(program, major, "Lack of %s operator\n\tInternal:%s-%u", __FILE__, __LINE__);
                        return -1;
                    }
                    else
                    {
                        list_destroy(response1);
                        return 1;
                    }
                }
            }

            if (cnt_response1 == 0)
            {
                if (node2 != NULL)
                {
                    semantic_error(program, major, "Lhs not result\n\tInternal:%s-%u", __FILE__, __LINE__);
                    return -1;
                }
                semantic_error(program, major, "Rhs not result\n\tInternal:%s-%u", __FILE__, __LINE__);
                return -1;
            }
        }

        list_destroy(response1);
    }

    region_step_2:
    {
        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_content(program, node2, response1, flag);
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

                int32_t r2 = f(program, node1, response2, flag);
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

                        int32_t r3 = semantic_dispart(program, item1, item2, major, OPERATOR_SIMBOL, response, flag);
                        if (r3 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r3 == 0)
                        {
                            semantic_error(program, major, "Lack of %s operator\n\tInternal:%s-%u", 
                                OPERATOR_SIMBOL, __FILE__, __LINE__);
                            return -1;
                        }
                        else
                        {
                            list_destroy(response2);
                            list_destroy(response1);
                            return 1;
                        }
                    }

                    if (cnt_response2 == 0)
                    {
                        semantic_error(program, major, "Lhs not result\n\tInternal:%s-%u", __FILE__, __LINE__);
                        return -1;
                    }
                }

                list_destroy(response2);
            }

            if (cnt_response1 == 0)
            {
                if (node2 != NULL)
                {
                    semantic_error(program, major, "Lhs not result\n\tInternal:%s-%u", __FILE__, __LINE__);
                    return -1;
                }
                semantic_error(program, major, "Rhs not result\n\tInternal:%s-%u", __FILE__, __LINE__);
                return -1;
            }
        }

        list_destroy(response1);
    }

    return 0;
}

static int32_t
semantic_unary(program_t *program, node_t *node, char *OPERATOR_SIMBOL, int32_t (*f)(program_t *, node_t *, list_t *, uint64_t), list_t *response, uint64_t flag)
{
    node_unary_t *unary1 = (node_unary_t *)node->value;

    return semantic_fsolve(program, unary1->right, NULL, node, OPERATOR_SIMBOL, f, response, flag);
}

static int32_t
semantic_binary(program_t *program, node_t *node, char *OPERATOR_SIMBOL, int32_t (*f)(program_t *, node_t *, list_t *, uint64_t), list_t *response, uint64_t flag)
{
    node_binary_t *binary1 = (node_binary_t *)node->value;

    return semantic_fsolve(program, binary1->left, binary1->right, node, OPERATOR_SIMBOL, f, response, flag);
}

static int32_t
semantic_prefix(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    if (node->kind == NODE_KIND_TILDE)
    {
        return semantic_unary(program, node, "~", semantic_prefix, response, flag);
    }
    else
    if (node->kind == NODE_KIND_POS)
    {
        return semantic_unary(program, node, "+", semantic_prefix, response, flag);
    }
    else
    if (node->kind == NODE_KIND_NEG)
    {
        return semantic_unary(program, node, "-", semantic_prefix, response, flag);
    }
    else
    if (node->kind == NODE_KIND_NOT)
    {
        return semantic_unary(program, node, "!", semantic_prefix, response, flag);
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
        return semantic_binary(program, node, "**", semantic_pow, response, flag);
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
        return semantic_binary(program, node, "*", semantic_multipicative, response, flag);
    }
    else
    if (node->kind == NODE_KIND_DIV)
    {
        return semantic_binary(program, node, "/", semantic_multipicative, response, flag);
    }
    else
    if (node->kind == NODE_KIND_MOD)
    {
        return semantic_binary(program, node, "%", semantic_multipicative, response, flag);
    }
    else
    if (node->kind == NODE_KIND_EPI)
    {
        return semantic_binary(program, node, "\\", semantic_multipicative, response, flag);
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
        return semantic_binary(program, node, "+", semantic_addative, response, flag);
    }
    else
    if (node->kind == NODE_KIND_MINUS)
    {
        return semantic_binary(program, node, "-", semantic_addative, response, flag);
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
        return semantic_binary(program, node, ">>", semantic_addative, response, flag);
    }
    else
    if (node->kind == NODE_KIND_SHL)
    {
        return semantic_binary(program, node, "<<", semantic_addative, response, flag);
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
        return semantic_binary(program, node, "<", semantic_shifting, response, flag);
    }
    else
    if (node->kind == NODE_KIND_LE)
    {
        return semantic_binary(program, node, "<=", semantic_shifting, response, flag);
    }
    else
    if (node->kind == NODE_KIND_GT)
    {
        return semantic_binary(program, node, ">", semantic_shifting, response, flag);
    }
    else
    if (node->kind == NODE_KIND_GE)
    {
        return semantic_binary(program, node, ">=", semantic_shifting, response, flag);
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
        return semantic_binary(program, node, "==", semantic_relational, response, flag);
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

        int32_t r1 = semantic_binary(program, node, "==", semantic_relational, response1, flag);
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

                int32_t r2 = semantic_dispart(program, item1, NULL, node, "!", response, flag);
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
        return semantic_binary(program, node, "&", semantic_bitwise_and, response, flag);
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
        return semantic_binary(program, node, "^", semantic_bitwise_xor, response, flag);
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
        return semantic_binary(program, node, "|", semantic_bitwise_or, response, flag);
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
        return semantic_binary(program, node, "&&", semantic_logical_and, response, flag);
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
        return semantic_binary(program, node, "||", semantic_logical_or, response, flag);
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

static int32_t
semantic_cselect(program_t *program, node_t *node, node_t *target, list_t *response, uint64_t flag)
{
    node_class_t *class1 = (node_class_t *)node->value;

    node_t *node1 = class1->block;
    node_block_t *block1 = (node_block_t *)node1->value;

    ilist_t *a1;
    for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        if (item1->kind == NODE_KIND_PROPERTY)
        {
            node_property_t *property1 = (node_property_t *)item1->value;
            if (semantic_idcmp(property1->key, target) == 1)
            {
                if ((property1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                {
                    semantic_error(program, target, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                        property1->key->position.path, property1->key->position.line, property1->key->position.column, __FILE__, __LINE__);
                    return -1;
                }

                ilist_t *il1 = list_rpush(response, item1);
                if (il1 == NULL)
                {
                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                if (heritage3->value_update == NULL)
                {
                    list_t *response2 = list_create();
                    if (response2 == NULL)
                    {
                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                    
                    int32_t r1 = semantic_resolve(program, heritage3->type->parent, heritage3->type, response2, SEMANTIC_FLAG_NONE);
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
                                if ((item3->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    semantic_error(program, heritage3->type, "Instance of object, in confronting with (%s-%lld:%lld)",
                                        item3->position.path, item3->position.line, item3->position.column);
                                    return -1;
                                }

                                node_t *clone1 = node_clone(item3->parent, item3);
                                if (clone1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                clone1->flag |= NODE_FLAG_INSTANCE;

                                heritage3->value_update = clone1;

                                int32_t r2 = semantic_cselect(program, clone1, target, response, flag);
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
                                if (generic1->type != NULL)
                                {
                                    list_t *response3 = list_create();
                                    if (response3 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_gresolve(program, item3, response3, SEMANTIC_FLAG_NONE);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    {
                                        uint64_t cnt_response3 = 0;

                                        ilist_t *a4;
                                        for (a4 = response3->begin;a4 != response3->end;a4 = a4->next)
                                        {
                                            cnt_response3 += 1;

                                            node_t *item4 = (node_t *)a4->value;

                                            if (item4->kind == NODE_KIND_CLASS)
                                            {
                                                node_t *clone1 = node_clone(item4->parent, item4);
                                                if (clone1 == NULL)
                                                {
                                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                clone1->flag |= NODE_FLAG_INSTANCE;

                                                heritage3->value_update = clone1;

                                                int32_t r2 = semantic_cselect(program, clone1, target, response, flag);
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

                                        if (cnt_response3 == 0)
                                        {
                                            semantic_error(program, generic1->key, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }

                                    list_destroy(response3);
                                }
                            }
                            else
                            {
                                semantic_error(program, heritage3->type, "Wrong type\n\tInternal:%s-%u",
                                    __FILE__, __LINE__);
                                return -1;
                            }
                            break;
                        }

                        if (cnt_response2 == 0)
                        {
                            semantic_error(program, heritage3->type, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    list_destroy(response2);
                }
                else
                {
                    node_t *value_update1 = heritage3->value_update;
                    if (value_update1->kind == NODE_KIND_CLASS)
                    {
                        int32_t r2 = semantic_cselect(program, value_update1, target, response, flag);
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

    return 0;
}

static int32_t
semantic_oselect(program_t *program, node_t *node, node_t *target, list_t *response, uint64_t flag)
{
    node_block_t *block1 = (node_block_t *)node->value;

    ilist_t *a1;
    for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        if (item1->kind == NODE_KIND_PROPERTY)
        {
            node_pair_t *pair1 = (node_pair_t *)item1->value;
            if (semantic_idcmp(pair1->key, target) == 1)
            {
                ilist_t *il1 = list_rpush(response, item1);
                if (il1 == NULL)
                {
                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                    return -1;
                }
                return 1;
            }
            continue;
        }
    }

    return 0;
}

int32_t
semantic_assignment(program_t *program, node_t *node1, node_t *node2, node_t *major, uint64_t flag)
{
    if (node1->kind == NODE_KIND_VAR)
    {
        node_var_t *var1 = (node_var_t *)node1->value;

        if (node2->kind == NODE_KIND_VAR)
        {
            node_var_t *var2 = (node_var_t *)node2->value;
            if (var2->value_update != NULL)
            {
                node_t *value_update2 = var2->value_update;
                if (value_update2->kind == NODE_KIND_LAMBDA)
                {
                    if ((value_update2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                    {
                        semantic_error(program, major, "Not an instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            value_update2->position.path, value_update2->position.line, value_update2->position.column, __FILE__, __LINE__);
                        return -1;
                    }

                    node_lambda_t *fun2 = (node_lambda_t *)value_update2->value;
                    if (var1->type != NULL)
                    {
                        node_t *type1 = var1->type;

                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                                if (item1->kind == NODE_KIND_LAMBDA)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    node_fn_t *fun1 = (node_fn_t *)item1->value;
                                    int32_t r2 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        int32_t r3 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                        if (r3 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r3 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        {
                                            list_destroy(response1);
                                            var1->value_update = var2->value_update;
                                            return 1;
                                        }
                                    }
                                }
                                else
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                            }

                            if (cnt_response1 == 0)
                            {
                                semantic_error(program, type1, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                    else
                    {
                        if (var1->value_update != NULL)
                        {
                            node_t *value_update1 = var1->value_update;
                            if (value_update1->kind == NODE_KIND_LAMBDA)
                            {
                                node_lambda_t *fun1 = (node_lambda_t *)value_update1->value;
                                int32_t r1 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                {
                                    int32_t r2 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        var1->value_update = value_update2;
                                        return 1;
                                    }
                                }
                            }
                            else
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                        else
                        {
                            list_t *response1 = list_create();
                            if (response1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                                    if (item1->kind == NODE_KIND_LAMBDA)
                                    {
                                        node_lambda_t *fun1 = (node_lambda_t *)item1->value;
                                        int32_t r2 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        {
                                            int32_t r3 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                            if (r3 == -1)
                                            {
                                                return -1;
                                            }
                                            else
                                            if (r3 == 0)
                                            {
                                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                return -1;
                                            }
                                            else
                                            {
                                                var1->value_update = value_update2;
                                                list_destroy(response1);
                                                return 1;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
                    }
                }
                else
                if (value_update2->kind == NODE_KIND_OBJECT)
                {
                    if ((value_update2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                    {
                        semantic_error(program, major, "Not an instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            value_update2->position.path, value_update2->position.line, value_update2->position.column, __FILE__, __LINE__);
                        return -1;
                    }

                    if (var1->type != NULL)
                    {
                        node_t *type1 = var1->type;

                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                                if (item1->kind == NODE_KIND_OBJECT)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_eqaul_otos(program, item1, value_update2);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        list_destroy(response1);
                                        var1->value_update = var2->value_update;
                                        return 1;
                                    }
                                }
                                else
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
                    else
                    {
                        if (var1->value_update != NULL)
                        {
                            node_t *value_update1 = var1->value_update;
                            if (value_update1->kind == NODE_KIND_OBJECT)
                            {
                                int32_t r1 = semantic_eqaul_osos(program, value_update1, value_update2);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                {
                                    var1->value_update = var2->value_update;
                                    return 1;
                                }
                            }
                            else
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                        else
                        {
                            list_t *response1 = list_create();
                            if (response1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                                    if (item1->kind == NODE_KIND_OBJECT)
                                    {
                                        int32_t r1 = semantic_eqaul_osos(program, item1, value_update2);
                                        if (r1 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r1 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        {
                                            var1->value_update = var2->value_update;
                                            return 1;
                                        }
                                    }
                                    else
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
                    }
                }
                else
                {
                    if (var1->type != NULL)
                    {
                        node_t *type1 = (node_t *)var1->type;

                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                                if (item1->kind == NODE_KIND_LAMBDA)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (item1->kind == NODE_KIND_OBJECT)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (item1->kind == NODE_KIND_CLASS)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of class, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_tequality(program, item1, value_update2);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    if (r2 == 1)
                                    {
                                        if (value_update2->kind == NODE_KIND_CLASS)
                                        {
                                            list_destroy(response1);
                                            var1->value_update = value_update2;
                                            return 1;
                                        }
                                        else
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }
                                }
                                else
                                if (item1->kind == NODE_KIND_GENERIC)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of class, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    
                                    node_generic_t *generic1 = (node_generic_t *)item1->value;
                                    if (generic1->type != NULL)
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
                                                    if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                    {
                                                        semantic_error(program, item1, "Instance of class, in confronting with (%s-%lld:%lld)",
                                                            item2->position.path, item2->position.line, item2->position.column);
                                                        return -1;
                                                    }

                                                    int32_t r3 = semantic_tequality(program, item2, value_update2);
                                                    if (r3 == -1)
                                                    {
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 0)
                                                    {
                                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 1)
                                                    {
                                                        if (value_update2->kind == NODE_KIND_CLASS)
                                                        {
                                                            list_destroy(response2);
                                                            list_destroy(response1);
                                                            var1->value_update = value_update2;
                                                            return 1;
                                                        }
                                                        else
                                                        {
                                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                    }
                                                }
                                                else
                                                if (item2->kind == NODE_KIND_GENERIC)
                                                {
                                                    if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                    {
                                                        semantic_error(program, item1, "Instance of generic, in confronting with (%s-%lld:%lld)",
                                                            item2->position.path, item2->position.line, item2->position.column);
                                                        return -1;
                                                    }

                                                    int32_t r3 = semantic_tequality(program, item2, value_update2);
                                                    if (r3 == -1)
                                                    {
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 0)
                                                    {
                                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 1)
                                                    {
                                                        if (value_update2->kind == NODE_KIND_GENERIC)
                                                        {
                                                            list_destroy(response2);
                                                            list_destroy(response1);
                                                            var1->value_update = value_update2;
                                                            return 1;
                                                        }
                                                        else
                                                        {
                                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                    }
                                                }
                                                else
                                                {
                                                    semantic_error(program, item1, "Not a valid type, in confronting with (%s-%lld:%lld)",
                                                        item2->position.path, item2->position.line, item2->position.column);
                                                    return -1;
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
                                        int32_t r2 = semantic_tequality(program, item1, value_update2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            if (value_update2->kind == NODE_KIND_GENERIC)
                                            {
                                                list_destroy(response1);
                                                var1->value_update = value_update2;
                                                return 1;
                                            }
                                            else
                                            {
                                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    semantic_error(program, type1, "Not a type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
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
                    else
                    {
                        if (var1->value_update != NULL)
                        {
                            node_t *value_update1 = var1->value_update;
                            if (value_update1->kind == NODE_KIND_LAMBDA)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            if (value_update1->kind == NODE_KIND_OBJECT)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            {
                                int32_t r2 = semantic_tequality(program, value_update1, value_update2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    var1->value_update = value_update2;
                                    return 1;
                                }
                            }
                        }
                        else
                        {
                            list_t *response1 = list_create();
                            if (response1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                                    if (item1->kind == NODE_KIND_LAMBDA)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    if (item1->kind == NODE_KIND_OBJECT)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        int32_t r2 = semantic_tequality(program, item1, value_update2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            var1->value_update = value_update2;
                                            return 1;
                                        }
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
                    }
                }
            }
            else
            {
                semantic_error(program, var2->key, "Unitialized\n\tInternal:%s-%u", __FILE__, __LINE__);
                return -1;
            }
        }
        else
        if (node2->kind == NODE_KIND_ENTITY)
        {
            node_entity_t *entity2 = (node_entity_t *)node2->value;
            if (entity2->value_update != NULL)
            {
                node_t *value_update2 = entity2->value_update;
                if (value_update2->kind == NODE_KIND_LAMBDA)
                {
                    if ((value_update2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                    {
                        semantic_error(program, major, "Not an instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            value_update2->position.path, value_update2->position.line, value_update2->position.column, __FILE__, __LINE__);
                        return -1;
                    }

                    node_lambda_t *fun2 = (node_lambda_t *)value_update2->value;
                    if (var1->type != NULL)
                    {
                        node_t *type1 = var1->type;

                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                                if (item1->kind == NODE_KIND_LAMBDA)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    node_fn_t *fun1 = (node_fn_t *)item1->value;
                                    int32_t r2 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        int32_t r3 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                        if (r3 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r3 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        {
                                            list_destroy(response1);
                                            var1->value_update = entity2->value_update;
                                            return 1;
                                        }
                                    }
                                }
                                else
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                            }

                            if (cnt_response1 == 0)
                            {
                                semantic_error(program, type1, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                    else
                    {
                        if (var1->value_update != NULL)
                        {
                            node_t *value_update1 = var1->value_update;
                            if (value_update1->kind == NODE_KIND_LAMBDA)
                            {
                                node_lambda_t *fun1 = (node_lambda_t *)value_update1->value;
                                int32_t r1 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                {
                                    int32_t r2 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        var1->value_update = value_update2;
                                        return 1;
                                    }
                                }
                            }
                            else
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                        else
                        {
                            list_t *response1 = list_create();
                            if (response1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                                    if (item1->kind == NODE_KIND_LAMBDA)
                                    {
                                        node_lambda_t *fun1 = (node_lambda_t *)item1->value;
                                        int32_t r2 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        {
                                            int32_t r3 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                            if (r3 == -1)
                                            {
                                                return -1;
                                            }
                                            else
                                            if (r3 == 0)
                                            {
                                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                return -1;
                                            }
                                            else
                                            {
                                                var1->value_update = value_update2;
                                                list_destroy(response1);
                                                return 1;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
                    }
                }
                else
                if (value_update2->kind == NODE_KIND_OBJECT)
                {
                    if ((value_update2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                    {
                        semantic_error(program, major, "Not an instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            value_update2->position.path, value_update2->position.line, value_update2->position.column, __FILE__, __LINE__);
                        return -1;
                    }

                    if (var1->type != NULL)
                    {
                        node_t *type1 = var1->type;

                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                                if (item1->kind == NODE_KIND_OBJECT)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_eqaul_otos(program, item1, value_update2);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        list_destroy(response1);
                                        var1->value_update = entity2->value_update;
                                        return 1;
                                    }
                                }
                                else
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
                    else
                    {
                        if (var1->value_update != NULL)
                        {
                            node_t *value_update1 = var1->value_update;
                            if (value_update1->kind == NODE_KIND_OBJECT)
                            {
                                int32_t r1 = semantic_eqaul_osos(program, value_update1, value_update2);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                {
                                    var1->value_update = entity2->value_update;
                                    return 1;
                                }
                            }
                            else
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                        else
                        {
                            list_t *response1 = list_create();
                            if (response1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                                    if (item1->kind == NODE_KIND_OBJECT)
                                    {
                                        int32_t r1 = semantic_eqaul_osos(program, item1, value_update2);
                                        if (r1 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r1 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        {
                                            var1->value_update = entity2->value_update;
                                            return 1;
                                        }
                                    }
                                    else
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
                    }
                }
                else
                {
                    if (var1->type != NULL)
                    {
                        node_t *type1 = (node_t *)var1->type;

                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                                if (item1->kind == NODE_KIND_LAMBDA)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (item1->kind == NODE_KIND_OBJECT)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (item1->kind == NODE_KIND_CLASS)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of class, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_tequality(program, item1, value_update2);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    if (r2 == 1)
                                    {
                                        if (value_update2->kind == NODE_KIND_CLASS)
                                        {
                                            list_destroy(response1);
                                            var1->value_update = value_update2;
                                            return 1;
                                        }
                                        else
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }
                                }
                                else
                                if (item1->kind == NODE_KIND_GENERIC)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of class, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    
                                    node_generic_t *generic1 = (node_generic_t *)item1->value;
                                    if (generic1->type != NULL)
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
                                                    if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                    {
                                                        semantic_error(program, item1, "Instance of class, in confronting with (%s-%lld:%lld)",
                                                            item2->position.path, item2->position.line, item2->position.column);
                                                        return -1;
                                                    }

                                                    int32_t r3 = semantic_tequality(program, item2, value_update2);
                                                    if (r3 == -1)
                                                    {
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 0)
                                                    {
                                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 1)
                                                    {
                                                        if (value_update2->kind == NODE_KIND_CLASS)
                                                        {
                                                            list_destroy(response2);
                                                            list_destroy(response1);
                                                            var1->value_update = value_update2;
                                                            return 1;
                                                        }
                                                        else
                                                        {
                                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                    }
                                                }
                                                else
                                                if (item2->kind == NODE_KIND_GENERIC)
                                                {
                                                    if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                    {
                                                        semantic_error(program, item1, "Instance of generic, in confronting with (%s-%lld:%lld)",
                                                            item2->position.path, item2->position.line, item2->position.column);
                                                        return -1;
                                                    }

                                                    int32_t r3 = semantic_tequality(program, item2, value_update2);
                                                    if (r3 == -1)
                                                    {
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 0)
                                                    {
                                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 1)
                                                    {
                                                        if (value_update2->kind == NODE_KIND_GENERIC)
                                                        {
                                                            list_destroy(response2);
                                                            list_destroy(response1);
                                                            var1->value_update = value_update2;
                                                            return 1;
                                                        }
                                                        else
                                                        {
                                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                    }
                                                }
                                                else
                                                {
                                                    semantic_error(program, item1, "Not a valid type, in confronting with (%s-%lld:%lld)",
                                                        item2->position.path, item2->position.line, item2->position.column);
                                                    return -1;
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
                                        int32_t r2 = semantic_tequality(program, item1, value_update2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            if (value_update2->kind == NODE_KIND_GENERIC)
                                            {
                                                list_destroy(response1);
                                                var1->value_update = value_update2;
                                                return 1;
                                            }
                                            else
                                            {
                                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    semantic_error(program, type1, "Not a type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
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
                    else
                    {
                        if (var1->value_update != NULL)
                        {
                            node_t *value_update1 = var1->value_update;
                            if (value_update1->kind == NODE_KIND_LAMBDA)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            if (value_update1->kind == NODE_KIND_OBJECT)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            {
                                int32_t r2 = semantic_tequality(program, value_update1, value_update2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    var1->value_update = value_update2;
                                    return 1;
                                }
                            }
                        }
                        else
                        {
                            list_t *response1 = list_create();
                            if (response1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                                    if (item1->kind == NODE_KIND_LAMBDA)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    if (item1->kind == NODE_KIND_OBJECT)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        int32_t r2 = semantic_tequality(program, item1, value_update2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            var1->value_update = value_update2;
                                            return 1;
                                        }
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
                    }
                }
            }
            else
            {
                semantic_error(program, entity2->key, "Unitialized\n\tInternal:%s-%u", __FILE__, __LINE__);
                return -1;
            }
        }
        else
        if (node2->kind == NODE_KIND_PROPERTY)
        {
            node_property_t *property2 = (node_property_t *)node2->value;
            if (property2->value_update != NULL)
            {
                node_t *value_update2 = property2->value_update;
                if (value_update2->kind == NODE_KIND_LAMBDA)
                {
                    if ((value_update2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                    {
                        semantic_error(program, major, "Not an instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            value_update2->position.path, value_update2->position.line, value_update2->position.column, __FILE__, __LINE__);
                        return -1;
                    }

                    node_lambda_t *fun2 = (node_lambda_t *)value_update2->value;
                    if (var1->type != NULL)
                    {
                        node_t *type1 = var1->type;

                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                                if (item1->kind == NODE_KIND_LAMBDA)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    node_fn_t *fun1 = (node_fn_t *)item1->value;
                                    int32_t r2 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        int32_t r3 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                        if (r3 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r3 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        {
                                            list_destroy(response1);
                                            var1->value_update = property2->value_update;
                                            return 1;
                                        }
                                    }
                                }
                                else
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                            }

                            if (cnt_response1 == 0)
                            {
                                semantic_error(program, type1, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                    else
                    {
                        if (var1->value_update != NULL)
                        {
                            node_t *value_update1 = var1->value_update;
                            if (value_update1->kind == NODE_KIND_LAMBDA)
                            {
                                node_lambda_t *fun1 = (node_lambda_t *)value_update1->value;
                                int32_t r1 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                {
                                    int32_t r2 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        var1->value_update = value_update2;
                                        return 1;
                                    }
                                }
                            }
                            else
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                        else
                        {
                            list_t *response1 = list_create();
                            if (response1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                                    if (item1->kind == NODE_KIND_LAMBDA)
                                    {
                                        node_lambda_t *fun1 = (node_lambda_t *)item1->value;
                                        int32_t r2 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        {
                                            int32_t r3 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                            if (r3 == -1)
                                            {
                                                return -1;
                                            }
                                            else
                                            if (r3 == 0)
                                            {
                                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                return -1;
                                            }
                                            else
                                            {
                                                var1->value_update = value_update2;
                                                list_destroy(response1);
                                                return 1;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
                    }
                }
                else
                if (value_update2->kind == NODE_KIND_OBJECT)
                {
                    if ((value_update2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                    {
                        semantic_error(program, major, "Not an instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            value_update2->position.path, value_update2->position.line, value_update2->position.column, __FILE__, __LINE__);
                        return -1;
                    }

                    if (var1->type != NULL)
                    {
                        node_t *type1 = var1->type;

                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                                if (item1->kind == NODE_KIND_OBJECT)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_eqaul_otos(program, item1, value_update2);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        list_destroy(response1);
                                        var1->value_update = property2->value_update;
                                        return 1;
                                    }
                                }
                                else
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
                    else
                    {
                        if (var1->value_update != NULL)
                        {
                            node_t *value_update1 = var1->value_update;
                            if (value_update1->kind == NODE_KIND_OBJECT)
                            {
                                int32_t r1 = semantic_eqaul_osos(program, value_update1, value_update2);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                {
                                    var1->value_update = property2->value_update;
                                    return 1;
                                }
                            }
                            else
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                        else
                        {
                            list_t *response1 = list_create();
                            if (response1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                                    if (item1->kind == NODE_KIND_OBJECT)
                                    {
                                        int32_t r1 = semantic_eqaul_osos(program, item1, value_update2);
                                        if (r1 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r1 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        {
                                            var1->value_update = property2->value_update;
                                            return 1;
                                        }
                                    }
                                    else
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
                    }
                }
                else
                {
                    if (var1->type != NULL)
                    {
                        node_t *type1 = (node_t *)var1->type;

                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                                if (item1->kind == NODE_KIND_LAMBDA)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (item1->kind == NODE_KIND_OBJECT)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (item1->kind == NODE_KIND_CLASS)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of class, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_tequality(program, item1, value_update2);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    if (r2 == 1)
                                    {
                                        if (value_update2->kind == NODE_KIND_CLASS)
                                        {
                                            list_destroy(response1);
                                            var1->value_update = value_update2;
                                            return 1;
                                        }
                                        else
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }
                                }
                                else
                                if (item1->kind == NODE_KIND_GENERIC)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of class, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    
                                    node_generic_t *generic1 = (node_generic_t *)item1->value;
                                    if (generic1->type != NULL)
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
                                                    if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                    {
                                                        semantic_error(program, item1, "Instance of class, in confronting with (%s-%lld:%lld)",
                                                            item2->position.path, item2->position.line, item2->position.column);
                                                        return -1;
                                                    }

                                                    int32_t r3 = semantic_tequality(program, item2, value_update2);
                                                    if (r3 == -1)
                                                    {
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 0)
                                                    {
                                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 1)
                                                    {
                                                        if (value_update2->kind == NODE_KIND_CLASS)
                                                        {
                                                            list_destroy(response2);
                                                            list_destroy(response1);
                                                            var1->value_update = value_update2;
                                                            return 1;
                                                        }
                                                        else
                                                        {
                                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                    }
                                                }
                                                else
                                                if (item2->kind == NODE_KIND_GENERIC)
                                                {
                                                    if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                    {
                                                        semantic_error(program, item1, "Instance of generic, in confronting with (%s-%lld:%lld)",
                                                            item2->position.path, item2->position.line, item2->position.column);
                                                        return -1;
                                                    }

                                                    int32_t r3 = semantic_tequality(program, item2, value_update2);
                                                    if (r3 == -1)
                                                    {
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 0)
                                                    {
                                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 1)
                                                    {
                                                        if (value_update2->kind == NODE_KIND_GENERIC)
                                                        {
                                                            list_destroy(response2);
                                                            list_destroy(response1);
                                                            var1->value_update = value_update2;
                                                            return 1;
                                                        }
                                                        else
                                                        {
                                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                    }
                                                }
                                                else
                                                {
                                                    semantic_error(program, item1, "Not a valid type, in confronting with (%s-%lld:%lld)",
                                                        item2->position.path, item2->position.line, item2->position.column);
                                                    return -1;
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
                                        int32_t r2 = semantic_tequality(program, item1, value_update2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            if (value_update2->kind == NODE_KIND_GENERIC)
                                            {
                                                list_destroy(response1);
                                                var1->value_update = value_update2;
                                                return 1;
                                            }
                                            else
                                            {
                                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    semantic_error(program, type1, "Not a type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
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
                    else
                    {
                        if (var1->value_update != NULL)
                        {
                            node_t *value_update1 = var1->value_update;
                            if (value_update1->kind == NODE_KIND_LAMBDA)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            if (value_update1->kind == NODE_KIND_OBJECT)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            {
                                int32_t r2 = semantic_tequality(program, value_update1, value_update2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    var1->value_update = value_update2;
                                    return 1;
                                }
                            }
                        }
                        else
                        {
                            list_t *response1 = list_create();
                            if (response1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                                    if (item1->kind == NODE_KIND_LAMBDA)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    if (item1->kind == NODE_KIND_OBJECT)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        int32_t r2 = semantic_tequality(program, item1, value_update2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            var1->value_update = value_update2;
                                            return 1;
                                        }
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
                    }
                }
            }
            else
            {
                if (property2->type != NULL)
                {
                    node_t *type2 = property2->type;

                    list_t *response1 = list_create();
                    if (response1 == NULL)
                    {
                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r1 = semantic_resolve(program, type2->parent, type2, response1, flag);
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
                                if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    semantic_error(program, type2, "Instance of class, in confronting with (%s-%lld:%lld)",
                                        item1->position.path, item1->position.line, item1->position.column);
                                    return -1;
                                }

                                node_t *clone1 = node_clone(item1->parent, item1);
                                if (clone1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                clone1->flag |= NODE_FLAG_INSTANCE;

                                property2->value_update = clone1;

                                list_destroy(response1);
                                return semantic_assignment(program, node1, clone1, major, flag);
                            }
                            else
                            if (item1->kind == NODE_KIND_GENERIC)
                            {
                                if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    semantic_error(program, type2, "Instance of generic, in confronting with (%s-%lld:%lld)",
                                        item1->position.path, item1->position.line, item1->position.column);
                                    return -1;
                                }

                                node_generic_t *generic1 = (node_generic_t *)item1->value;
                                if (generic1->type != NULL)
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
                                                if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                {
                                                    semantic_error(program, item1, "Instance of class, in confronting with (%s-%lld:%lld)",
                                                        item2->position.path, item2->position.line, item2->position.column);
                                                    return -1;
                                                }

                                                node_t *clone1 = node_clone(item2->parent, item2);
                                                if (clone1 == NULL)
                                                {
                                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                clone1->flag |= NODE_FLAG_INSTANCE;

                                                property2->value_update = clone1;

                                                list_destroy(response1);
                                                return semantic_assignment(program, node1, clone1, major, flag);
                                            }
                                            else
                                            if (item2->kind == NODE_KIND_GENERIC)
                                            {
                                                if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                {
                                                    semantic_error(program, item1, "Instance of generic, in confronting with (%s-%lld:%lld)",
                                                        item2->position.path, item2->position.line, item2->position.column);
                                                    return -1;
                                                }

                                                node_t *clone1 = node_clone(item2->parent, item2);
                                                if (clone1 == NULL)
                                                {
                                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                clone1->flag |= NODE_FLAG_INSTANCE;

                                                property2->value_update = clone1;

                                                list_destroy(response1);
                                                return semantic_assignment(program, node1, clone1, major, flag);
                                            }
                                            else
                                            {
                                                semantic_error(program, item1, "Not a valid type, in confronting with (%s-%lld:%lld)",
                                                    item2->position.path, item2->position.line, item2->position.column);
                                                return -1;
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
                                    node_t *clone1 = node_clone(item1->parent, item1);
                                    if (clone1 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    clone1->flag |= NODE_FLAG_INSTANCE;

                                    property2->value_update = clone1;

                                    list_destroy(response1);
                                    return semantic_assignment(program, node1, clone1, major, flag);
                                }
                            }
                            else
                            if (item1->kind == NODE_KIND_LAMBDA)
                            {
                                if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    semantic_error(program, type2, "Instance of fun, in confronting with (%s-%lld:%lld)",
                                        item1->position.path, item1->position.line, item1->position.column);
                                    return -1;
                                }

                                node_t *clone1 = node_clone(item1->parent, item1);
                                if (clone1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                clone1->flag |= NODE_FLAG_INSTANCE;

                                property2->value_update = clone1;

                                list_destroy(response1);
                                return semantic_assignment(program, node1, clone1, major, flag);
                            }
                            else
                            if (item1->kind == NODE_KIND_OBJECT)
                            {
                                if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    semantic_error(program, type2, "Instance of object, in confronting with (%s-%lld:%lld)",
                                        item1->position.path, item1->position.line, item1->position.column);
                                    return -1;
                                }

                                node_t *clone1 = node_clone(item1->parent, item1);
                                if (clone1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                clone1->flag |= NODE_FLAG_INSTANCE;

                                property2->value_update = clone1;

                                list_destroy(response1);
                                return semantic_assignment(program, node1, clone1, major, flag);
                            }
                            else
                            {
                                semantic_error(program, type2, "Not a type, in confronting with (%s-%lld:%lld)",
                                    item1->position.path, item1->position.line, item1->position.column);
                                return -1;
                            }
                        }

                        if (cnt_response1 == 0)
                        {
                            semantic_error(program, type2, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    list_destroy(response1);
                }
                else
                {
                    list_t *response1 = list_create();
                    if (response1 == NULL)
                    {
                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r1 = semantic_expression(program, property2->value, response1, flag);
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
                            
                            int32_t r2 = semantic_assignment(program, node2, item1, node2, flag);
                            if (r2 == -1)
                            {
                                return -1;
                            }

                            list_destroy(response1);
                            return semantic_assignment(program, node1, item1, major, flag);
                        }

                        if (cnt_response1 == 0)
                        {
                            semantic_error(program, property2->value, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    list_destroy(response1);
                }
            }
        }
        else
        if (node2->kind == NODE_KIND_PARAMETER)
        {
            node_parameter_t *parameter2 = (node_parameter_t *)node2->value;
            if (parameter2->value_update != NULL)
            {
                node_t *value_update2 = parameter2->value_update;
                if (value_update2->kind == NODE_KIND_LAMBDA)
                {
                    if ((value_update2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                    {
                        semantic_error(program, major, "Not an instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            value_update2->position.path, value_update2->position.line, value_update2->position.column, __FILE__, __LINE__);
                        return -1;
                    }

                    node_lambda_t *fun2 = (node_lambda_t *)value_update2->value;
                    if (var1->type != NULL)
                    {
                        node_t *type1 = var1->type;

                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                                if (item1->kind == NODE_KIND_LAMBDA)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    node_fn_t *fun1 = (node_fn_t *)item1->value;
                                    int32_t r2 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        int32_t r3 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                        if (r3 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r3 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        {
                                            list_destroy(response1);
                                            var1->value_update = parameter2->value_update;
                                            return 1;
                                        }
                                    }
                                }
                                else
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                            }

                            if (cnt_response1 == 0)
                            {
                                semantic_error(program, type1, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                    else
                    {
                        if (var1->value_update != NULL)
                        {
                            node_t *value_update1 = var1->value_update;
                            if (value_update1->kind == NODE_KIND_LAMBDA)
                            {
                                node_lambda_t *fun1 = (node_lambda_t *)value_update1->value;
                                int32_t r1 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                {
                                    int32_t r2 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        var1->value_update = value_update2;
                                        return 1;
                                    }
                                }
                            }
                            else
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                        else
                        {
                            list_t *response1 = list_create();
                            if (response1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                                    if (item1->kind == NODE_KIND_LAMBDA)
                                    {
                                        node_lambda_t *fun1 = (node_lambda_t *)item1->value;
                                        int32_t r2 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        {
                                            int32_t r3 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                            if (r3 == -1)
                                            {
                                                return -1;
                                            }
                                            else
                                            if (r3 == 0)
                                            {
                                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                return -1;
                                            }
                                            else
                                            {
                                                var1->value_update = value_update2;
                                                list_destroy(response1);
                                                return 1;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
                    }
                }
                else
                if (value_update2->kind == NODE_KIND_OBJECT)
                {
                    if ((value_update2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                    {
                        semantic_error(program, major, "Not an instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            value_update2->position.path, value_update2->position.line, value_update2->position.column, __FILE__, __LINE__);
                        return -1;
                    }

                    if (var1->type != NULL)
                    {
                        node_t *type1 = var1->type;

                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                                if (item1->kind == NODE_KIND_OBJECT)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_eqaul_otos(program, item1, value_update2);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        list_destroy(response1);
                                        var1->value_update = parameter2->value_update;
                                        return 1;
                                    }
                                }
                                else
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
                    else
                    {
                        if (var1->value_update != NULL)
                        {
                            node_t *value_update1 = var1->value_update;
                            if (value_update1->kind == NODE_KIND_OBJECT)
                            {
                                int32_t r1 = semantic_eqaul_osos(program, value_update1, value_update2);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                {
                                    var1->value_update = parameter2->value_update;
                                    return 1;
                                }
                            }
                            else
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                        else
                        {
                            list_t *response1 = list_create();
                            if (response1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                                    if (item1->kind == NODE_KIND_OBJECT)
                                    {
                                        int32_t r1 = semantic_eqaul_osos(program, item1, value_update2);
                                        if (r1 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r1 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        {
                                            var1->value_update = parameter2->value_update;
                                            return 1;
                                        }
                                    }
                                    else
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
                    }
                }
                else
                {
                    if (var1->type != NULL)
                    {
                        node_t *type1 = (node_t *)var1->type;

                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                                if (item1->kind == NODE_KIND_LAMBDA)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (item1->kind == NODE_KIND_OBJECT)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (item1->kind == NODE_KIND_CLASS)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of class, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_tequality(program, item1, value_update2);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    if (r2 == 1)
                                    {
                                        if (value_update2->kind == NODE_KIND_CLASS)
                                        {
                                            list_destroy(response1);
                                            var1->value_update = value_update2;
                                            return 1;
                                        }
                                        else
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }
                                }
                                else
                                if (item1->kind == NODE_KIND_GENERIC)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of class, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    
                                    node_generic_t *generic1 = (node_generic_t *)item1->value;
                                    if (generic1->type != NULL)
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
                                                    if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                    {
                                                        semantic_error(program, item1, "Instance of class, in confronting with (%s-%lld:%lld)",
                                                            item2->position.path, item2->position.line, item2->position.column);
                                                        return -1;
                                                    }

                                                    int32_t r3 = semantic_tequality(program, item2, value_update2);
                                                    if (r3 == -1)
                                                    {
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 0)
                                                    {
                                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 1)
                                                    {
                                                        if (value_update2->kind == NODE_KIND_CLASS)
                                                        {
                                                            list_destroy(response2);
                                                            list_destroy(response1);
                                                            var1->value_update = value_update2;
                                                            return 1;
                                                        }
                                                        else
                                                        {
                                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                    }
                                                }
                                                else
                                                if (item2->kind == NODE_KIND_GENERIC)
                                                {
                                                    if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                    {
                                                        semantic_error(program, item1, "Instance of generic, in confronting with (%s-%lld:%lld)",
                                                            item2->position.path, item2->position.line, item2->position.column);
                                                        return -1;
                                                    }

                                                    int32_t r3 = semantic_tequality(program, item2, value_update2);
                                                    if (r3 == -1)
                                                    {
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 0)
                                                    {
                                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 1)
                                                    {
                                                        if (value_update2->kind == NODE_KIND_GENERIC)
                                                        {
                                                            list_destroy(response2);
                                                            list_destroy(response1);
                                                            var1->value_update = value_update2;
                                                            return 1;
                                                        }
                                                        else
                                                        {
                                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                    }
                                                }
                                                else
                                                {
                                                    semantic_error(program, item1, "Not a valid type, in confronting with (%s-%lld:%lld)",
                                                        item2->position.path, item2->position.line, item2->position.column);
                                                    return -1;
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
                                        int32_t r2 = semantic_tequality(program, item1, value_update2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            if (value_update2->kind == NODE_KIND_GENERIC)
                                            {
                                                list_destroy(response1);
                                                var1->value_update = value_update2;
                                                return 1;
                                            }
                                            else
                                            {
                                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    semantic_error(program, type1, "Not a type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
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
                    else
                    {
                        if (var1->value_update != NULL)
                        {
                            node_t *value_update1 = var1->value_update;
                            if (value_update1->kind == NODE_KIND_LAMBDA)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            if (value_update1->kind == NODE_KIND_OBJECT)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            {
                                int32_t r2 = semantic_tequality(program, value_update1, value_update2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    var1->value_update = value_update2;
                                    return 1;
                                }
                            }
                        }
                        else
                        {
                            list_t *response1 = list_create();
                            if (response1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                                    if (item1->kind == NODE_KIND_LAMBDA)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    if (item1->kind == NODE_KIND_OBJECT)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        int32_t r2 = semantic_tequality(program, item1, value_update2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            var1->value_update = value_update2;
                                            return 1;
                                        }
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
                    }
                }
            }
            else
            {
                if (parameter2->type != NULL)
                {
                    node_t *type2 = parameter2->type;

                    list_t *response1 = list_create();
                    if (response1 == NULL)
                    {
                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r1 = semantic_resolve(program, type2->parent, type2, response1, flag);
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
                                if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    semantic_error(program, type2, "Instance of class, in confronting with (%s-%lld:%lld)",
                                        item1->position.path, item1->position.line, item1->position.column);
                                    return -1;
                                }

                                node_t *clone1 = node_clone(item1->parent, item1);
                                if (clone1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                clone1->flag |= NODE_FLAG_INSTANCE;

                                parameter2->value_update = clone1;

                                list_destroy(response1);
                                return semantic_assignment(program, node1, clone1, major, flag);
                            }
                            else
                            if (item1->kind == NODE_KIND_GENERIC)
                            {
                                if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    semantic_error(program, type2, "Instance of generic, in confronting with (%s-%lld:%lld)",
                                        item1->position.path, item1->position.line, item1->position.column);
                                    return -1;
                                }

                                node_generic_t *generic1 = (node_generic_t *)item1->value;
                                if (generic1->type != NULL)
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
                                                if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                {
                                                    semantic_error(program, item1, "Instance of class, in confronting with (%s-%lld:%lld)",
                                                        item2->position.path, item2->position.line, item2->position.column);
                                                    return -1;
                                                }

                                                node_t *clone1 = node_clone(item2->parent, item2);
                                                if (clone1 == NULL)
                                                {
                                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                clone1->flag |= NODE_FLAG_INSTANCE;

                                                parameter2->value_update = clone1;

                                                list_destroy(response1);
                                                return semantic_assignment(program, node1, clone1, major, flag);
                                            }
                                            else
                                            if (item2->kind == NODE_KIND_GENERIC)
                                            {
                                                if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                {
                                                    semantic_error(program, item1, "Instance of generic, in confronting with (%s-%lld:%lld)",
                                                        item2->position.path, item2->position.line, item2->position.column);
                                                    return -1;
                                                }

                                                node_t *clone1 = node_clone(item2->parent, item2);
                                                if (clone1 == NULL)
                                                {
                                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                clone1->flag |= NODE_FLAG_INSTANCE;

                                                parameter2->value_update = clone1;

                                                list_destroy(response1);
                                                return semantic_assignment(program, node1, clone1, major, flag);
                                            }
                                            else
                                            {
                                                semantic_error(program, item1, "Not a valid type, in confronting with (%s-%lld:%lld)",
                                                    item2->position.path, item2->position.line, item2->position.column);
                                                return -1;
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
                                    node_t *clone1 = node_clone(item1->parent, item1);
                                    if (clone1 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    clone1->flag |= NODE_FLAG_INSTANCE;

                                    parameter2->value_update = clone1;

                                    list_destroy(response1);
                                    return semantic_assignment(program, node1, clone1, major, flag);
                                }
                            }
                            else
                            if (item1->kind == NODE_KIND_LAMBDA)
                            {
                                if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    semantic_error(program, type2, "Instance of fun, in confronting with (%s-%lld:%lld)",
                                        item1->position.path, item1->position.line, item1->position.column);
                                    return -1;
                                }

                                node_t *clone1 = node_clone(item1->parent, item1);
                                if (clone1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                clone1->flag |= NODE_FLAG_INSTANCE;

                                parameter2->value_update = clone1;

                                list_destroy(response1);
                                return semantic_assignment(program, node1, clone1, major, flag);
                            }
                            else
                            if (item1->kind == NODE_KIND_OBJECT)
                            {
                                if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    semantic_error(program, type2, "Instance of object, in confronting with (%s-%lld:%lld)",
                                        item1->position.path, item1->position.line, item1->position.column);
                                    return -1;
                                }

                                node_t *clone1 = node_clone(item1->parent, item1);
                                if (clone1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                clone1->flag |= NODE_FLAG_INSTANCE;

                                parameter2->value_update = clone1;

                                list_destroy(response1);
                                return semantic_assignment(program, node1, clone1, major, flag);
                            }
                            else
                            {
                                semantic_error(program, type2, "Not a type, in confronting with (%s-%lld:%lld)",
                                    item1->position.path, item1->position.line, item1->position.column);
                                return -1;
                            }
                        }

                        if (cnt_response1 == 0)
                        {
                            semantic_error(program, type2, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    list_destroy(response1);
                }
                else
                {
                    list_t *response1 = list_create();
                    if (response1 == NULL)
                    {
                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r1 = semantic_expression(program, parameter2->value, response1, flag);
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
                            
                            int32_t r2 = semantic_assignment(program, node2, item1, node2, flag);
                            if (r2 == -1)
                            {
                                return -1;
                            }

                            list_destroy(response1);
                            return semantic_assignment(program, node1, item1, major, flag);
                        }

                        if (cnt_response1 == 0)
                        {
                            semantic_error(program, parameter2->value, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    list_destroy(response1);
                }
            }
        }
        else
        if (node2->kind == NODE_KIND_CLASS)
        {
            if ((node2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
            {
                semantic_error(program, major, "Not an instance of class, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                return -1;
            }

            if (var1->type != NULL)
            {
                node_t *type1 = (node_t *)var1->type;

                list_t *response1 = list_create();
                if (response1 == NULL)
                {
                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                        if (item1->kind == NODE_KIND_LAMBDA)
                        {
                            if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                            {
                                semantic_error(program, major, "Instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                return -1;
                            }

                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                        else
                        if (item1->kind == NODE_KIND_OBJECT)
                        {
                            if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                            {
                                semantic_error(program, major, "Instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            
                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                        else
                        if (item1->kind == NODE_KIND_CLASS)
                        {
                            if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                            {
                                semantic_error(program, major, "Instance of class, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r2 = semantic_tequality(program, item1, node2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 0)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                list_destroy(response1);
                                var1->value_update = node2;
                                return 1;
                            }
                        }
                        else
                        if (item1->kind == NODE_KIND_GENERIC)
                        {
                            if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                            {
                                semantic_error(program, major, "Instance of class, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            
                            node_generic_t *generic1 = (node_generic_t *)item1->value;
                            if (generic1->type != NULL)
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
                                            if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                            {
                                                semantic_error(program, item1, "Instance of class, in confronting with (%s-%lld:%lld)",
                                                    item2->position.path, item2->position.line, item2->position.column);
                                                return -1;
                                            }

                                            int32_t r3 = semantic_tequality(program, item2, node2);
                                            if (r3 == -1)
                                            {
                                                return -1;
                                            }
                                            else
                                            if (r3 == 0)
                                            {
                                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                return -1;
                                            }
                                            else
                                            if (r3 == 1)
                                            {
                                                list_destroy(response2);
                                                list_destroy(response1);
                                                var1->value_update = node2;
                                                return 1;
                                            }
                                        }
                                        else
                                        if (item2->kind == NODE_KIND_GENERIC)
                                        {
                                            if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                            {
                                                semantic_error(program, item1, "Instance of generic, in confronting with (%s-%lld:%lld)",
                                                    item2->position.path, item2->position.line, item2->position.column);
                                                return -1;
                                            }

                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        {
                                            semantic_error(program, item1, "Not a valid type, in confronting with (%s-%lld:%lld)",
                                                item2->position.path, item2->position.line, item2->position.column);
                                            return -1;
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
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                        else
                        {
                            semantic_error(program, type1, "Not a type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    if (cnt_response1 == 0)
                    {
                        semantic_error(program, type1, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                        return -1;
                    }
                }
            }
            else
            {
                if (var1->value_update != NULL)
                {
                    node_t *value_update1 = var1->value_update;
                    if (value_update1->kind == NODE_KIND_LAMBDA)
                    {
                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                        return -1;
                    }
                    else
                    if (value_update1->kind == NODE_KIND_OBJECT)
                    {
                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                        return -1;
                    }
                    else
                    {
                        int32_t r2 = semantic_tequality(program, value_update1, node2);
                        if (r2 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r2 == 0)
                        {
                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                        else
                        if (r2 == 1)
                        {
                            var1->value_update = node2;
                            return 1;
                        }
                    }
                }
                else
                {
                    list_t *response1 = list_create();
                    if (response1 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                            if (item1->kind == NODE_KIND_LAMBDA)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            if (item1->kind == NODE_KIND_OBJECT)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            {
                                int32_t r2 = semantic_tequality(program, item1, node2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    var1->value_update = node2;
                                    return 1;
                                }
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
            }
        }
        else
        if (node2->kind == NODE_KIND_GENERIC)
        {
            if ((node2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
            {
                semantic_error(program, major, "Not an instance of generic, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                return -1;
            }

            if (var1->type != NULL)
            {
                node_t *type1 = (node_t *)var1->type;

                list_t *response1 = list_create();
                if (response1 == NULL)
                {
                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                        if (item1->kind == NODE_KIND_LAMBDA)
                        {
                            if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                            {
                                semantic_error(program, major, "Instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                return -1;
                            }

                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                        else
                        if (item1->kind == NODE_KIND_OBJECT)
                        {
                            if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                            {
                                semantic_error(program, major, "Instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            
                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                        else
                        if (item1->kind == NODE_KIND_CLASS)
                        {
                            if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                            {
                                semantic_error(program, major, "Instance of class, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                return -1;
                            }

                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                        else
                        if (item1->kind == NODE_KIND_GENERIC)
                        {
                            if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                            {
                                semantic_error(program, major, "Instance of class, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            
                            node_generic_t *generic1 = (node_generic_t *)item1->value;
                            if (generic1->type != NULL)
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
                                            if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                            {
                                                semantic_error(program, item1, "Instance of class, in confronting with (%s-%lld:%lld)",
                                                    item2->position.path, item2->position.line, item2->position.column);
                                                return -1;
                                            }

                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        if (item2->kind == NODE_KIND_GENERIC)
                                        {
                                            if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                            {
                                                semantic_error(program, item1, "Instance of generic, in confronting with (%s-%lld:%lld)",
                                                    item2->position.path, item2->position.line, item2->position.column);
                                                return -1;
                                            }

                                            int32_t r3 = semantic_tequality(program, item2, node2);
                                            if (r3 == -1)
                                            {
                                                return -1;
                                            }
                                            else
                                            if (r3 == 0)
                                            {
                                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                return -1;
                                            }
                                            else
                                            if (r3 == 1)
                                            {
                                                list_destroy(response2);
                                                list_destroy(response1);
                                                var1->value_update = node2;
                                                return 1;
                                            }
                                        }
                                        else
                                        {
                                            semantic_error(program, item1, "Not a valid type, in confronting with (%s-%lld:%lld)",
                                                item2->position.path, item2->position.line, item2->position.column);
                                            return -1;
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
                                int32_t r3 = semantic_tequality(program, item1, node2);
                                if (r3 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r3 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (r3 == 1)
                                {
                                    list_destroy(response1);
                                    var1->value_update = node2;
                                    return 1;
                                }
                            }
                        }
                        else
                        {
                            semantic_error(program, type1, "Not a type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    if (cnt_response1 == 0)
                    {
                        semantic_error(program, type1, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                        return -1;
                    }
                }
            }
            else
            {
                if (var1->value_update != NULL)
                {
                    node_t *value_update1 = var1->value_update;
                    if (value_update1->kind == NODE_KIND_LAMBDA)
                    {
                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                        return -1;
                    }
                    else
                    if (value_update1->kind == NODE_KIND_OBJECT)
                    {
                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                        return -1;
                    }
                    else
                    {
                        int32_t r2 = semantic_tequality(program, value_update1, node2);
                        if (r2 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r2 == 0)
                        {
                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                        else
                        if (r2 == 1)
                        {
                            var1->value_update = node2;
                            return 1;
                        }
                    }
                }
                else
                {
                    list_t *response1 = list_create();
                    if (response1 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                            if (item1->kind == NODE_KIND_LAMBDA)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            if (item1->kind == NODE_KIND_OBJECT)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            {
                                int32_t r2 = semantic_tequality(program, item1, node2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    var1->value_update = node2;
                                    return 1;
                                }
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
            }
        }
        else
        if (node2->kind == NODE_KIND_LAMBDA)
        {
            node_lambda_t *fun2 = (node_lambda_t *)node2->value;
            if (var1->type != NULL)
            {
                node_t *type1 = var1->type;

                list_t *response1 = list_create();
                if (response1 == NULL)
                {
                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                        if (item1->kind == NODE_KIND_LAMBDA)
                        {
                            if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                            {
                                semantic_error(program, major, "Instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                return -1;
                            }

                            node_fn_t *fun1 = (node_fn_t *)item1->value;
                            int32_t r2 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 0)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            {
                                int32_t r3 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                if (r3 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r3 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                {
                                    list_destroy(response1);
                                    var1->value_update = node2;
                                    return 1;
                                }
                            }
                        }
                        else
                        {
                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    if (cnt_response1 == 0)
                    {
                        semantic_error(program, type1, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                        return -1;
                    }
                }
            }
            else
            {
                if (var1->value_update != NULL)
                {
                    node_t *value_update1 = var1->value_update;
                    if (value_update1->kind == NODE_KIND_LAMBDA)
                    {
                        node_lambda_t *fun1 = (node_lambda_t *)value_update1->value;
                        int32_t r1 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 0)
                        {
                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                        else
                        {
                            int32_t r2 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 0)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            {
                                var1->value_update = node2;
                                return 1;
                            }
                        }
                    }
                    else
                    {
                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                        return -1;
                    }
                }
                else
                {
                    list_t *response1 = list_create();
                    if (response1 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                            if (item1->kind == NODE_KIND_LAMBDA)
                            {
                                node_lambda_t *fun1 = (node_lambda_t *)item1->value;
                                int32_t r2 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                {
                                    int32_t r3 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                    if (r3 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r3 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        var1->value_update = node2;
                                        list_destroy(response1);
                                        return 1;
                                    }
                                }
                            }
                            else
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
            }
        }
        else
        if (node2->kind == NODE_KIND_OBJECT)
        {
            if ((node2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
            {
                semantic_error(program, major, "Not an instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                return -1;
            }

            if (var1->type != NULL)
            {
                node_t *type1 = var1->type;

                list_t *response1 = list_create();
                if (response1 == NULL)
                {
                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                        if (item1->kind == NODE_KIND_OBJECT)
                        {
                            if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                            {
                                semantic_error(program, major, "Instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r2 = semantic_eqaul_otos(program, item1, node2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 0)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            {
                                list_destroy(response1);
                                var1->value_update = node2;
                                return 1;
                            }
                        }
                        else
                        {
                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
            else
            {
                if (var1->value_update != NULL)
                {
                    node_t *value_update1 = var1->value_update;
                    if (value_update1->kind == NODE_KIND_OBJECT)
                    {
                        int32_t r1 = semantic_eqaul_osos(program, value_update1, node2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 0)
                        {
                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                        else
                        {
                            var1->value_update = node2;
                            return 1;
                        }
                    }
                    else
                    {
                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                        return -1;
                    }
                }
                else
                {
                    list_t *response1 = list_create();
                    if (response1 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                            if (item1->kind == NODE_KIND_OBJECT)
                            {
                                int32_t r1 = semantic_eqaul_osos(program, item1, node2);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                {
                                    var1->value_update = node2;
                                    return 1;
                                }
                            }
                            else
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
            }
        }
        else
        {
            semantic_error(program, major, "Not valid value, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
            return -1;
        }
    }
    else
    if (node1->kind == NODE_KIND_PROPERTY)
    {
        node_property_t *property1 = (node_property_t *)node1->value;

        if (node2->kind == NODE_KIND_VAR)
        {
            node_var_t *var2 = (node_var_t *)node2->value;
            if (var2->value_update != NULL)
            {
                node_t *value_update2 = var2->value_update;
                if (value_update2->kind == NODE_KIND_LAMBDA)
                {
                    if ((value_update2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                    {
                        semantic_error(program, major, "Not an instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            value_update2->position.path, value_update2->position.line, value_update2->position.column, __FILE__, __LINE__);
                        return -1;
                    }

                    node_lambda_t *fun2 = (node_lambda_t *)value_update2->value;
                    if (property1->type != NULL)
                    {
                        node_t *type1 = property1->type;

                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                                if (item1->kind == NODE_KIND_LAMBDA)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    node_fn_t *fun1 = (node_fn_t *)item1->value;
                                    int32_t r2 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        int32_t r3 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                        if (r3 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r3 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        {
                                            list_destroy(response1);
                                            property1->value_update = var2->value_update;
                                            return 1;
                                        }
                                    }
                                }
                                else
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                            }

                            if (cnt_response1 == 0)
                            {
                                semantic_error(program, type1, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                    else
                    {
                        if (property1->value_update != NULL)
                        {
                            node_t *value_update1 = property1->value_update;
                            if (value_update1->kind == NODE_KIND_LAMBDA)
                            {
                                node_lambda_t *fun1 = (node_lambda_t *)value_update1->value;
                                int32_t r1 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                {
                                    int32_t r2 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        property1->value_update = value_update2;
                                        return 1;
                                    }
                                }
                            }
                            else
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                        else
                        {
                            list_t *response1 = list_create();
                            if (response1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                                    if (item1->kind == NODE_KIND_LAMBDA)
                                    {
                                        node_lambda_t *fun1 = (node_lambda_t *)item1->value;
                                        int32_t r2 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        {
                                            int32_t r3 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                            if (r3 == -1)
                                            {
                                                return -1;
                                            }
                                            else
                                            if (r3 == 0)
                                            {
                                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                return -1;
                                            }
                                            else
                                            {
                                                property1->value_update = value_update2;
                                                list_destroy(response1);
                                                return 1;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
                    }
                }
                else
                if (value_update2->kind == NODE_KIND_OBJECT)
                {
                    if ((value_update2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                    {
                        semantic_error(program, major, "Not an instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            value_update2->position.path, value_update2->position.line, value_update2->position.column, __FILE__, __LINE__);
                        return -1;
                    }

                    if (property1->type != NULL)
                    {
                        node_t *type1 = property1->type;

                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                                if (item1->kind == NODE_KIND_OBJECT)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_eqaul_otos(program, item1, value_update2);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        list_destroy(response1);
                                        property1->value_update = var2->value_update;
                                        return 1;
                                    }
                                }
                                else
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
                    else
                    {
                        if (property1->value_update != NULL)
                        {
                            node_t *value_update1 = property1->value_update;
                            if (value_update1->kind == NODE_KIND_OBJECT)
                            {
                                int32_t r1 = semantic_eqaul_osos(program, value_update1, value_update2);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                {
                                    property1->value_update = var2->value_update;
                                    return 1;
                                }
                            }
                            else
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                        else
                        {
                            list_t *response1 = list_create();
                            if (response1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                                    if (item1->kind == NODE_KIND_OBJECT)
                                    {
                                        int32_t r1 = semantic_eqaul_osos(program, item1, value_update2);
                                        if (r1 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r1 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        {
                                            property1->value_update = var2->value_update;
                                            return 1;
                                        }
                                    }
                                    else
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
                    }
                }
                else
                {
                    if (property1->type != NULL)
                    {
                        node_t *type1 = (node_t *)property1->type;

                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                                if (item1->kind == NODE_KIND_LAMBDA)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (item1->kind == NODE_KIND_OBJECT)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (item1->kind == NODE_KIND_CLASS)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of class, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_tequality(program, item1, value_update2);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    if (r2 == 1)
                                    {
                                        if (value_update2->kind == NODE_KIND_CLASS)
                                        {
                                            list_destroy(response1);
                                            property1->value_update = value_update2;
                                            return 1;
                                        }
                                        else
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }
                                }
                                else
                                if (item1->kind == NODE_KIND_GENERIC)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of class, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    
                                    node_generic_t *generic1 = (node_generic_t *)item1->value;
                                    if (generic1->type != NULL)
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
                                                    if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                    {
                                                        semantic_error(program, item1, "Instance of class, in confronting with (%s-%lld:%lld)",
                                                            item2->position.path, item2->position.line, item2->position.column);
                                                        return -1;
                                                    }

                                                    int32_t r3 = semantic_tequality(program, item2, value_update2);
                                                    if (r3 == -1)
                                                    {
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 0)
                                                    {
                                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 1)
                                                    {
                                                        if (value_update2->kind == NODE_KIND_CLASS)
                                                        {
                                                            list_destroy(response2);
                                                            list_destroy(response1);
                                                            property1->value_update = value_update2;
                                                            return 1;
                                                        }
                                                        else
                                                        {
                                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                    }
                                                }
                                                else
                                                if (item2->kind == NODE_KIND_GENERIC)
                                                {
                                                    if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                    {
                                                        semantic_error(program, item1, "Instance of generic, in confronting with (%s-%lld:%lld)",
                                                            item2->position.path, item2->position.line, item2->position.column);
                                                        return -1;
                                                    }

                                                    int32_t r3 = semantic_tequality(program, item2, value_update2);
                                                    if (r3 == -1)
                                                    {
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 0)
                                                    {
                                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 1)
                                                    {
                                                        if (value_update2->kind == NODE_KIND_GENERIC)
                                                        {
                                                            list_destroy(response2);
                                                            list_destroy(response1);
                                                            property1->value_update = value_update2;
                                                            return 1;
                                                        }
                                                        else
                                                        {
                                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                    }
                                                }
                                                else
                                                {
                                                    semantic_error(program, item1, "Not a valid type, in confronting with (%s-%lld:%lld)",
                                                        item2->position.path, item2->position.line, item2->position.column);
                                                    return -1;
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
                                        int32_t r2 = semantic_tequality(program, item1, value_update2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            if (value_update2->kind == NODE_KIND_GENERIC)
                                            {
                                                list_destroy(response1);
                                                property1->value_update = value_update2;
                                                return 1;
                                            }
                                            else
                                            {
                                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    semantic_error(program, type1, "Not a type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
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
                    else
                    {
                        if (property1->value_update != NULL)
                        {
                            node_t *value_update1 = property1->value_update;
                            if (value_update1->kind == NODE_KIND_LAMBDA)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            if (value_update1->kind == NODE_KIND_OBJECT)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            {
                                int32_t r2 = semantic_tequality(program, value_update1, value_update2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    property1->value_update = value_update2;
                                    return 1;
                                }
                            }
                        }
                        else
                        {
                            list_t *response1 = list_create();
                            if (response1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                                    if (item1->kind == NODE_KIND_LAMBDA)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    if (item1->kind == NODE_KIND_OBJECT)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        int32_t r2 = semantic_tequality(program, item1, value_update2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            property1->value_update = value_update2;
                                            return 1;
                                        }
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
                    }
                }
            }
            else
            {
                semantic_error(program, var2->key, "Unitialized\n\tInternal:%s-%u", __FILE__, __LINE__);
                return -1;
            }
        }
        else
        if (node2->kind == NODE_KIND_ENTITY)
        {
            node_entity_t *entity2 = (node_entity_t *)node2->value;
            if (entity2->value_update != NULL)
            {
                node_t *value_update2 = entity2->value_update;
                if (value_update2->kind == NODE_KIND_LAMBDA)
                {
                    if ((value_update2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                    {
                        semantic_error(program, major, "Not an instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            value_update2->position.path, value_update2->position.line, value_update2->position.column, __FILE__, __LINE__);
                        return -1;
                    }

                    node_lambda_t *fun2 = (node_lambda_t *)value_update2->value;
                    if (property1->type != NULL)
                    {
                        node_t *type1 = property1->type;

                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                                if (item1->kind == NODE_KIND_LAMBDA)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    node_fn_t *fun1 = (node_fn_t *)item1->value;
                                    int32_t r2 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        int32_t r3 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                        if (r3 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r3 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        {
                                            list_destroy(response1);
                                            property1->value_update = entity2->value_update;
                                            return 1;
                                        }
                                    }
                                }
                                else
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                            }

                            if (cnt_response1 == 0)
                            {
                                semantic_error(program, type1, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                    else
                    {
                        if (property1->value_update != NULL)
                        {
                            node_t *value_update1 = property1->value_update;
                            if (value_update1->kind == NODE_KIND_LAMBDA)
                            {
                                node_lambda_t *fun1 = (node_lambda_t *)value_update1->value;
                                int32_t r1 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                {
                                    int32_t r2 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        property1->value_update = value_update2;
                                        return 1;
                                    }
                                }
                            }
                            else
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                        else
                        {
                            list_t *response1 = list_create();
                            if (response1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                                    if (item1->kind == NODE_KIND_LAMBDA)
                                    {
                                        node_lambda_t *fun1 = (node_lambda_t *)item1->value;
                                        int32_t r2 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        {
                                            int32_t r3 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                            if (r3 == -1)
                                            {
                                                return -1;
                                            }
                                            else
                                            if (r3 == 0)
                                            {
                                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                return -1;
                                            }
                                            else
                                            {
                                                property1->value_update = value_update2;
                                                list_destroy(response1);
                                                return 1;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
                    }
                }
                else
                if (value_update2->kind == NODE_KIND_OBJECT)
                {
                    if ((value_update2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                    {
                        semantic_error(program, major, "Not an instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            value_update2->position.path, value_update2->position.line, value_update2->position.column, __FILE__, __LINE__);
                        return -1;
                    }

                    if (property1->type != NULL)
                    {
                        node_t *type1 = property1->type;

                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                                if (item1->kind == NODE_KIND_OBJECT)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_eqaul_otos(program, item1, value_update2);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        list_destroy(response1);
                                        property1->value_update = entity2->value_update;
                                        return 1;
                                    }
                                }
                                else
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
                    else
                    {
                        if (property1->value_update != NULL)
                        {
                            node_t *value_update1 = property1->value_update;
                            if (value_update1->kind == NODE_KIND_OBJECT)
                            {
                                int32_t r1 = semantic_eqaul_osos(program, value_update1, value_update2);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                {
                                    property1->value_update = entity2->value_update;
                                    return 1;
                                }
                            }
                            else
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                        else
                        {
                            list_t *response1 = list_create();
                            if (response1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                                    if (item1->kind == NODE_KIND_OBJECT)
                                    {
                                        int32_t r1 = semantic_eqaul_osos(program, item1, value_update2);
                                        if (r1 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r1 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        {
                                            property1->value_update = entity2->value_update;
                                            return 1;
                                        }
                                    }
                                    else
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
                    }
                }
                else
                {
                    if (property1->type != NULL)
                    {
                        node_t *type1 = (node_t *)property1->type;

                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                                if (item1->kind == NODE_KIND_LAMBDA)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (item1->kind == NODE_KIND_OBJECT)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (item1->kind == NODE_KIND_CLASS)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of class, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_tequality(program, item1, value_update2);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    if (r2 == 1)
                                    {
                                        if (value_update2->kind == NODE_KIND_CLASS)
                                        {
                                            list_destroy(response1);
                                            property1->value_update = value_update2;
                                            return 1;
                                        }
                                        else
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }
                                }
                                else
                                if (item1->kind == NODE_KIND_GENERIC)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of class, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    
                                    node_generic_t *generic1 = (node_generic_t *)item1->value;
                                    if (generic1->type != NULL)
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
                                                    if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                    {
                                                        semantic_error(program, item1, "Instance of class, in confronting with (%s-%lld:%lld)",
                                                            item2->position.path, item2->position.line, item2->position.column);
                                                        return -1;
                                                    }

                                                    int32_t r3 = semantic_tequality(program, item2, value_update2);
                                                    if (r3 == -1)
                                                    {
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 0)
                                                    {
                                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 1)
                                                    {
                                                        if (value_update2->kind == NODE_KIND_CLASS)
                                                        {
                                                            list_destroy(response2);
                                                            list_destroy(response1);
                                                            property1->value_update = value_update2;
                                                            return 1;
                                                        }
                                                        else
                                                        {
                                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                    }
                                                }
                                                else
                                                if (item2->kind == NODE_KIND_GENERIC)
                                                {
                                                    if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                    {
                                                        semantic_error(program, item1, "Instance of generic, in confronting with (%s-%lld:%lld)",
                                                            item2->position.path, item2->position.line, item2->position.column);
                                                        return -1;
                                                    }

                                                    int32_t r3 = semantic_tequality(program, item2, value_update2);
                                                    if (r3 == -1)
                                                    {
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 0)
                                                    {
                                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 1)
                                                    {
                                                        if (value_update2->kind == NODE_KIND_GENERIC)
                                                        {
                                                            list_destroy(response2);
                                                            list_destroy(response1);
                                                            property1->value_update = value_update2;
                                                            return 1;
                                                        }
                                                        else
                                                        {
                                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                    }
                                                }
                                                else
                                                {
                                                    semantic_error(program, item1, "Not a valid type, in confronting with (%s-%lld:%lld)",
                                                        item2->position.path, item2->position.line, item2->position.column);
                                                    return -1;
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
                                        int32_t r2 = semantic_tequality(program, item1, value_update2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            if (value_update2->kind == NODE_KIND_GENERIC)
                                            {
                                                list_destroy(response1);
                                                property1->value_update = value_update2;
                                                return 1;
                                            }
                                            else
                                            {
                                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    semantic_error(program, type1, "Not a type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
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
                    else
                    {
                        if (property1->value_update != NULL)
                        {
                            node_t *value_update1 = property1->value_update;
                            if (value_update1->kind == NODE_KIND_LAMBDA)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            if (value_update1->kind == NODE_KIND_OBJECT)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            {
                                int32_t r2 = semantic_tequality(program, value_update1, value_update2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    property1->value_update = value_update2;
                                    return 1;
                                }
                            }
                        }
                        else
                        {
                            list_t *response1 = list_create();
                            if (response1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                                    if (item1->kind == NODE_KIND_LAMBDA)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    if (item1->kind == NODE_KIND_OBJECT)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        int32_t r2 = semantic_tequality(program, item1, value_update2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            property1->value_update = value_update2;
                                            return 1;
                                        }
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
                    }
                }
            }
            else
            {
                semantic_error(program, entity2->key, "Unitialized\n\tInternal:%s-%u", __FILE__, __LINE__);
                return -1;
            }
        }
        else
        if (node2->kind == NODE_KIND_PROPERTY)
        {
            node_property_t *property2 = (node_property_t *)node2->value;
            if (property2->value_update != NULL)
            {
                node_t *value_update2 = property2->value_update;
                if (value_update2->kind == NODE_KIND_LAMBDA)
                {
                    if ((value_update2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                    {
                        semantic_error(program, major, "Not an instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            value_update2->position.path, value_update2->position.line, value_update2->position.column, __FILE__, __LINE__);
                        return -1;
                    }

                    node_lambda_t *fun2 = (node_lambda_t *)value_update2->value;
                    if (property1->type != NULL)
                    {
                        node_t *type1 = property1->type;

                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                                if (item1->kind == NODE_KIND_LAMBDA)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    node_fn_t *fun1 = (node_fn_t *)item1->value;
                                    int32_t r2 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        int32_t r3 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                        if (r3 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r3 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        {
                                            list_destroy(response1);
                                            property1->value_update = property2->value_update;
                                            return 1;
                                        }
                                    }
                                }
                                else
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                            }

                            if (cnt_response1 == 0)
                            {
                                semantic_error(program, type1, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                    else
                    {
                        if (property1->value_update != NULL)
                        {
                            node_t *value_update1 = property1->value_update;
                            if (value_update1->kind == NODE_KIND_LAMBDA)
                            {
                                node_lambda_t *fun1 = (node_lambda_t *)value_update1->value;
                                int32_t r1 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                {
                                    int32_t r2 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        property1->value_update = value_update2;
                                        return 1;
                                    }
                                }
                            }
                            else
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                        else
                        {
                            list_t *response1 = list_create();
                            if (response1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                                    if (item1->kind == NODE_KIND_LAMBDA)
                                    {
                                        node_lambda_t *fun1 = (node_lambda_t *)item1->value;
                                        int32_t r2 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        {
                                            int32_t r3 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                            if (r3 == -1)
                                            {
                                                return -1;
                                            }
                                            else
                                            if (r3 == 0)
                                            {
                                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                return -1;
                                            }
                                            else
                                            {
                                                property1->value_update = value_update2;
                                                list_destroy(response1);
                                                return 1;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
                    }
                }
                else
                if (value_update2->kind == NODE_KIND_OBJECT)
                {
                    if ((value_update2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                    {
                        semantic_error(program, major, "Not an instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            value_update2->position.path, value_update2->position.line, value_update2->position.column, __FILE__, __LINE__);
                        return -1;
                    }

                    if (property1->type != NULL)
                    {
                        node_t *type1 = property1->type;

                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                                if (item1->kind == NODE_KIND_OBJECT)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_eqaul_otos(program, item1, value_update2);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        list_destroy(response1);
                                        property1->value_update = property2->value_update;
                                        return 1;
                                    }
                                }
                                else
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
                    else
                    {
                        if (property1->value_update != NULL)
                        {
                            node_t *value_update1 = property1->value_update;
                            if (value_update1->kind == NODE_KIND_OBJECT)
                            {
                                int32_t r1 = semantic_eqaul_osos(program, value_update1, value_update2);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                {
                                    property1->value_update = property2->value_update;
                                    return 1;
                                }
                            }
                            else
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                        else
                        {
                            list_t *response1 = list_create();
                            if (response1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                                    if (item1->kind == NODE_KIND_OBJECT)
                                    {
                                        int32_t r1 = semantic_eqaul_osos(program, item1, value_update2);
                                        if (r1 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r1 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        {
                                            property1->value_update = property2->value_update;
                                            return 1;
                                        }
                                    }
                                    else
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
                    }
                }
                else
                {
                    if (property1->type != NULL)
                    {
                        node_t *type1 = (node_t *)property1->type;

                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                                if (item1->kind == NODE_KIND_LAMBDA)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (item1->kind == NODE_KIND_OBJECT)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (item1->kind == NODE_KIND_CLASS)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of class, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_tequality(program, item1, value_update2);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    if (r2 == 1)
                                    {
                                        if (value_update2->kind == NODE_KIND_CLASS)
                                        {
                                            list_destroy(response1);
                                            property1->value_update = value_update2;
                                            return 1;
                                        }
                                        else
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }
                                }
                                else
                                if (item1->kind == NODE_KIND_GENERIC)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of class, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    
                                    node_generic_t *generic1 = (node_generic_t *)item1->value;
                                    if (generic1->type != NULL)
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
                                                    if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                    {
                                                        semantic_error(program, item1, "Instance of class, in confronting with (%s-%lld:%lld)",
                                                            item2->position.path, item2->position.line, item2->position.column);
                                                        return -1;
                                                    }

                                                    int32_t r3 = semantic_tequality(program, item2, value_update2);
                                                    if (r3 == -1)
                                                    {
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 0)
                                                    {
                                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 1)
                                                    {
                                                        if (value_update2->kind == NODE_KIND_CLASS)
                                                        {
                                                            list_destroy(response2);
                                                            list_destroy(response1);
                                                            property1->value_update = value_update2;
                                                            return 1;
                                                        }
                                                        else
                                                        {
                                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                    }
                                                }
                                                else
                                                if (item2->kind == NODE_KIND_GENERIC)
                                                {
                                                    if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                    {
                                                        semantic_error(program, item1, "Instance of generic, in confronting with (%s-%lld:%lld)",
                                                            item2->position.path, item2->position.line, item2->position.column);
                                                        return -1;
                                                    }

                                                    int32_t r3 = semantic_tequality(program, item2, value_update2);
                                                    if (r3 == -1)
                                                    {
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 0)
                                                    {
                                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 1)
                                                    {
                                                        if (value_update2->kind == NODE_KIND_GENERIC)
                                                        {
                                                            list_destroy(response2);
                                                            list_destroy(response1);
                                                            property1->value_update = value_update2;
                                                            return 1;
                                                        }
                                                        else
                                                        {
                                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                    }
                                                }
                                                else
                                                {
                                                    semantic_error(program, item1, "Not a valid type, in confronting with (%s-%lld:%lld)",
                                                        item2->position.path, item2->position.line, item2->position.column);
                                                    return -1;
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
                                        int32_t r2 = semantic_tequality(program, item1, value_update2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            if (value_update2->kind == NODE_KIND_GENERIC)
                                            {
                                                list_destroy(response1);
                                                property1->value_update = value_update2;
                                                return 1;
                                            }
                                            else
                                            {
                                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    semantic_error(program, type1, "Not a type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
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
                    else
                    {
                        if (property1->value_update != NULL)
                        {
                            node_t *value_update1 = property1->value_update;
                            if (value_update1->kind == NODE_KIND_LAMBDA)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            if (value_update1->kind == NODE_KIND_OBJECT)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            {
                                int32_t r2 = semantic_tequality(program, value_update1, value_update2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    property1->value_update = value_update2;
                                    return 1;
                                }
                            }
                        }
                        else
                        {
                            list_t *response1 = list_create();
                            if (response1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                                    if (item1->kind == NODE_KIND_LAMBDA)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    if (item1->kind == NODE_KIND_OBJECT)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        int32_t r2 = semantic_tequality(program, item1, value_update2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            property1->value_update = value_update2;
                                            return 1;
                                        }
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
                    }
                }
            }
            else
            {
                if (property2->type != NULL)
                {
                    node_t *type2 = property2->type;

                    list_t *response1 = list_create();
                    if (response1 == NULL)
                    {
                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r1 = semantic_resolve(program, type2->parent, type2, response1, flag);
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
                                if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    semantic_error(program, type2, "Instance of class, in confronting with (%s-%lld:%lld)",
                                        item1->position.path, item1->position.line, item1->position.column);
                                    return -1;
                                }

                                node_t *clone1 = node_clone(item1->parent, item1);
                                if (clone1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                clone1->flag |= NODE_FLAG_INSTANCE;

                                property2->value_update = clone1;

                                list_destroy(response1);
                                return semantic_assignment(program, node1, clone1, major, flag);
                            }
                            else
                            if (item1->kind == NODE_KIND_GENERIC)
                            {
                                if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    semantic_error(program, type2, "Instance of generic, in confronting with (%s-%lld:%lld)",
                                        item1->position.path, item1->position.line, item1->position.column);
                                    return -1;
                                }

                                node_generic_t *generic1 = (node_generic_t *)item1->value;
                                if (generic1->type != NULL)
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
                                                if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                {
                                                    semantic_error(program, item1, "Instance of class, in confronting with (%s-%lld:%lld)",
                                                        item2->position.path, item2->position.line, item2->position.column);
                                                    return -1;
                                                }

                                                node_t *clone1 = node_clone(item2->parent, item2);
                                                if (clone1 == NULL)
                                                {
                                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                clone1->flag |= NODE_FLAG_INSTANCE;

                                                property2->value_update = clone1;

                                                list_destroy(response1);
                                                return semantic_assignment(program, node1, clone1, major, flag);
                                            }
                                            else
                                            if (item2->kind == NODE_KIND_GENERIC)
                                            {
                                                if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                {
                                                    semantic_error(program, item1, "Instance of generic, in confronting with (%s-%lld:%lld)",
                                                        item2->position.path, item2->position.line, item2->position.column);
                                                    return -1;
                                                }

                                                node_t *clone1 = node_clone(item2->parent, item2);
                                                if (clone1 == NULL)
                                                {
                                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                clone1->flag |= NODE_FLAG_INSTANCE;

                                                property2->value_update = clone1;

                                                list_destroy(response1);
                                                return semantic_assignment(program, node1, clone1, major, flag);
                                            }
                                            else
                                            {
                                                semantic_error(program, item1, "Not a valid type, in confronting with (%s-%lld:%lld)",
                                                    item2->position.path, item2->position.line, item2->position.column);
                                                return -1;
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
                                    node_t *clone1 = node_clone(item1->parent, item1);
                                    if (clone1 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    clone1->flag |= NODE_FLAG_INSTANCE;

                                    property2->value_update = clone1;

                                    list_destroy(response1);
                                    return semantic_assignment(program, node1, clone1, major, flag);
                                }
                            }
                            else
                            if (item1->kind == NODE_KIND_LAMBDA)
                            {
                                if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    semantic_error(program, type2, "Instance of fun, in confronting with (%s-%lld:%lld)",
                                        item1->position.path, item1->position.line, item1->position.column);
                                    return -1;
                                }

                                node_t *clone1 = node_clone(item1->parent, item1);
                                if (clone1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                clone1->flag |= NODE_FLAG_INSTANCE;

                                property2->value_update = clone1;

                                list_destroy(response1);
                                return semantic_assignment(program, node1, clone1, major, flag);
                            }
                            else
                            if (item1->kind == NODE_KIND_OBJECT)
                            {
                                if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    semantic_error(program, type2, "Instance of object, in confronting with (%s-%lld:%lld)",
                                        item1->position.path, item1->position.line, item1->position.column);
                                    return -1;
                                }

                                node_t *clone1 = node_clone(item1->parent, item1);
                                if (clone1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                clone1->flag |= NODE_FLAG_INSTANCE;

                                property2->value_update = clone1;

                                list_destroy(response1);
                                return semantic_assignment(program, node1, clone1, major, flag);
                            }
                            else
                            {
                                semantic_error(program, type2, "Not a type, in confronting with (%s-%lld:%lld)",
                                    item1->position.path, item1->position.line, item1->position.column);
                                return -1;
                            }
                        }

                        if (cnt_response1 == 0)
                        {
                            semantic_error(program, type2, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    list_destroy(response1);
                }
                else
                {
                    list_t *response1 = list_create();
                    if (response1 == NULL)
                    {
                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r1 = semantic_expression(program, property2->value, response1, flag);
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
                            
                            int32_t r2 = semantic_assignment(program, node2, item1, node2, flag);
                            if (r2 == -1)
                            {
                                return -1;
                            }

                            list_destroy(response1);
                            return semantic_assignment(program, node1, item1, major, flag);
                        }

                        if (cnt_response1 == 0)
                        {
                            semantic_error(program, property2->value, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    list_destroy(response1);
                }
            }
        }
        else
        if (node2->kind == NODE_KIND_PARAMETER)
        {
            node_parameter_t *parameter2 = (node_parameter_t *)node2->value;
            if (parameter2->value_update != NULL)
            {
                node_t *value_update2 = parameter2->value_update;
                if (value_update2->kind == NODE_KIND_LAMBDA)
                {
                    if ((value_update2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                    {
                        semantic_error(program, major, "Not an instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            value_update2->position.path, value_update2->position.line, value_update2->position.column, __FILE__, __LINE__);
                        return -1;
                    }

                    node_lambda_t *fun2 = (node_lambda_t *)value_update2->value;
                    if (property1->type != NULL)
                    {
                        node_t *type1 = property1->type;

                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                                if (item1->kind == NODE_KIND_LAMBDA)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    node_fn_t *fun1 = (node_fn_t *)item1->value;
                                    int32_t r2 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        int32_t r3 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                        if (r3 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r3 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        {
                                            list_destroy(response1);
                                            property1->value_update = parameter2->value_update;
                                            return 1;
                                        }
                                    }
                                }
                                else
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                            }

                            if (cnt_response1 == 0)
                            {
                                semantic_error(program, type1, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                    else
                    {
                        if (property1->value_update != NULL)
                        {
                            node_t *value_update1 = property1->value_update;
                            if (value_update1->kind == NODE_KIND_LAMBDA)
                            {
                                node_lambda_t *fun1 = (node_lambda_t *)value_update1->value;
                                int32_t r1 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                {
                                    int32_t r2 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        property1->value_update = value_update2;
                                        return 1;
                                    }
                                }
                            }
                            else
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                        else
                        {
                            list_t *response1 = list_create();
                            if (response1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                                    if (item1->kind == NODE_KIND_LAMBDA)
                                    {
                                        node_lambda_t *fun1 = (node_lambda_t *)item1->value;
                                        int32_t r2 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        {
                                            int32_t r3 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                            if (r3 == -1)
                                            {
                                                return -1;
                                            }
                                            else
                                            if (r3 == 0)
                                            {
                                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                return -1;
                                            }
                                            else
                                            {
                                                property1->value_update = value_update2;
                                                list_destroy(response1);
                                                return 1;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
                    }
                }
                else
                if (value_update2->kind == NODE_KIND_OBJECT)
                {
                    if ((value_update2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                    {
                        semantic_error(program, major, "Not an instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            value_update2->position.path, value_update2->position.line, value_update2->position.column, __FILE__, __LINE__);
                        return -1;
                    }

                    if (property1->type != NULL)
                    {
                        node_t *type1 = property1->type;

                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                                if (item1->kind == NODE_KIND_OBJECT)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_eqaul_otos(program, item1, value_update2);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        list_destroy(response1);
                                        property1->value_update = parameter2->value_update;
                                        return 1;
                                    }
                                }
                                else
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
                    else
                    {
                        if (property1->value_update != NULL)
                        {
                            node_t *value_update1 = property1->value_update;
                            if (value_update1->kind == NODE_KIND_OBJECT)
                            {
                                int32_t r1 = semantic_eqaul_osos(program, value_update1, value_update2);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                {
                                    property1->value_update = parameter2->value_update;
                                    return 1;
                                }
                            }
                            else
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                        else
                        {
                            list_t *response1 = list_create();
                            if (response1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                                    if (item1->kind == NODE_KIND_OBJECT)
                                    {
                                        int32_t r1 = semantic_eqaul_osos(program, item1, value_update2);
                                        if (r1 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r1 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        {
                                            property1->value_update = parameter2->value_update;
                                            return 1;
                                        }
                                    }
                                    else
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
                    }
                }
                else
                {
                    if (property1->type != NULL)
                    {
                        node_t *type1 = (node_t *)property1->type;

                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                                if (item1->kind == NODE_KIND_LAMBDA)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (item1->kind == NODE_KIND_OBJECT)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (item1->kind == NODE_KIND_CLASS)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of class, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_tequality(program, item1, value_update2);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    if (r2 == 1)
                                    {
                                        if (value_update2->kind == NODE_KIND_CLASS)
                                        {
                                            list_destroy(response1);
                                            property1->value_update = value_update2;
                                            return 1;
                                        }
                                        else
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }
                                }
                                else
                                if (item1->kind == NODE_KIND_GENERIC)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of class, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    
                                    node_generic_t *generic1 = (node_generic_t *)item1->value;
                                    if (generic1->type != NULL)
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
                                                    if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                    {
                                                        semantic_error(program, item1, "Instance of class, in confronting with (%s-%lld:%lld)",
                                                            item2->position.path, item2->position.line, item2->position.column);
                                                        return -1;
                                                    }

                                                    int32_t r3 = semantic_tequality(program, item2, value_update2);
                                                    if (r3 == -1)
                                                    {
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 0)
                                                    {
                                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 1)
                                                    {
                                                        if (value_update2->kind == NODE_KIND_CLASS)
                                                        {
                                                            list_destroy(response2);
                                                            list_destroy(response1);
                                                            property1->value_update = value_update2;
                                                            return 1;
                                                        }
                                                        else
                                                        {
                                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                    }
                                                }
                                                else
                                                if (item2->kind == NODE_KIND_GENERIC)
                                                {
                                                    if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                    {
                                                        semantic_error(program, item1, "Instance of generic, in confronting with (%s-%lld:%lld)",
                                                            item2->position.path, item2->position.line, item2->position.column);
                                                        return -1;
                                                    }

                                                    int32_t r3 = semantic_tequality(program, item2, value_update2);
                                                    if (r3 == -1)
                                                    {
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 0)
                                                    {
                                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 1)
                                                    {
                                                        if (value_update2->kind == NODE_KIND_GENERIC)
                                                        {
                                                            list_destroy(response2);
                                                            list_destroy(response1);
                                                            property1->value_update = value_update2;
                                                            return 1;
                                                        }
                                                        else
                                                        {
                                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                    }
                                                }
                                                else
                                                {
                                                    semantic_error(program, item1, "Not a valid type, in confronting with (%s-%lld:%lld)",
                                                        item2->position.path, item2->position.line, item2->position.column);
                                                    return -1;
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
                                        int32_t r2 = semantic_tequality(program, item1, value_update2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            if (value_update2->kind == NODE_KIND_GENERIC)
                                            {
                                                list_destroy(response1);
                                                property1->value_update = value_update2;
                                                return 1;
                                            }
                                            else
                                            {
                                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    semantic_error(program, type1, "Not a type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
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
                    else
                    {
                        if (property1->value_update != NULL)
                        {
                            node_t *value_update1 = property1->value_update;
                            if (value_update1->kind == NODE_KIND_LAMBDA)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            if (value_update1->kind == NODE_KIND_OBJECT)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            {
                                int32_t r2 = semantic_tequality(program, value_update1, value_update2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    property1->value_update = value_update2;
                                    return 1;
                                }
                            }
                        }
                        else
                        {
                            list_t *response1 = list_create();
                            if (response1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                                    if (item1->kind == NODE_KIND_LAMBDA)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    if (item1->kind == NODE_KIND_OBJECT)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        int32_t r2 = semantic_tequality(program, item1, value_update2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            property1->value_update = value_update2;
                                            return 1;
                                        }
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
                    }
                }
            }
            else
            {
                if (parameter2->type != NULL)
                {
                    node_t *type2 = parameter2->type;

                    list_t *response1 = list_create();
                    if (response1 == NULL)
                    {
                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r1 = semantic_resolve(program, type2->parent, type2, response1, flag);
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
                                if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    semantic_error(program, type2, "Instance of class, in confronting with (%s-%lld:%lld)",
                                        item1->position.path, item1->position.line, item1->position.column);
                                    return -1;
                                }

                                node_t *clone1 = node_clone(item1->parent, item1);
                                if (clone1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                clone1->flag |= NODE_FLAG_INSTANCE;

                                parameter2->value_update = clone1;

                                list_destroy(response1);
                                return semantic_assignment(program, node1, clone1, major, flag);
                            }
                            else
                            if (item1->kind == NODE_KIND_GENERIC)
                            {
                                if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    semantic_error(program, type2, "Instance of generic, in confronting with (%s-%lld:%lld)",
                                        item1->position.path, item1->position.line, item1->position.column);
                                    return -1;
                                }

                                node_generic_t *generic1 = (node_generic_t *)item1->value;
                                if (generic1->type != NULL)
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
                                                if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                {
                                                    semantic_error(program, item1, "Instance of class, in confronting with (%s-%lld:%lld)",
                                                        item2->position.path, item2->position.line, item2->position.column);
                                                    return -1;
                                                }

                                                node_t *clone1 = node_clone(item2->parent, item2);
                                                if (clone1 == NULL)
                                                {
                                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                clone1->flag |= NODE_FLAG_INSTANCE;

                                                parameter2->value_update = clone1;

                                                list_destroy(response1);
                                                return semantic_assignment(program, node1, clone1, major, flag);
                                            }
                                            else
                                            if (item2->kind == NODE_KIND_GENERIC)
                                            {
                                                if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                {
                                                    semantic_error(program, item1, "Instance of generic, in confronting with (%s-%lld:%lld)",
                                                        item2->position.path, item2->position.line, item2->position.column);
                                                    return -1;
                                                }

                                                node_t *clone1 = node_clone(item2->parent, item2);
                                                if (clone1 == NULL)
                                                {
                                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                clone1->flag |= NODE_FLAG_INSTANCE;

                                                parameter2->value_update = clone1;

                                                list_destroy(response1);
                                                return semantic_assignment(program, node1, clone1, major, flag);
                                            }
                                            else
                                            {
                                                semantic_error(program, item1, "Not a valid type, in confronting with (%s-%lld:%lld)",
                                                    item2->position.path, item2->position.line, item2->position.column);
                                                return -1;
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
                                    node_t *clone1 = node_clone(item1->parent, item1);
                                    if (clone1 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    clone1->flag |= NODE_FLAG_INSTANCE;

                                    parameter2->value_update = clone1;

                                    list_destroy(response1);
                                    return semantic_assignment(program, node1, clone1, major, flag);
                                }
                            }
                            else
                            if (item1->kind == NODE_KIND_LAMBDA)
                            {
                                if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    semantic_error(program, type2, "Instance of fun, in confronting with (%s-%lld:%lld)",
                                        item1->position.path, item1->position.line, item1->position.column);
                                    return -1;
                                }

                                node_t *clone1 = node_clone(item1->parent, item1);
                                if (clone1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                clone1->flag |= NODE_FLAG_INSTANCE;

                                parameter2->value_update = clone1;

                                list_destroy(response1);
                                return semantic_assignment(program, node1, clone1, major, flag);
                            }
                            else
                            if (item1->kind == NODE_KIND_OBJECT)
                            {
                                if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    semantic_error(program, type2, "Instance of object, in confronting with (%s-%lld:%lld)",
                                        item1->position.path, item1->position.line, item1->position.column);
                                    return -1;
                                }

                                node_t *clone1 = node_clone(item1->parent, item1);
                                if (clone1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                clone1->flag |= NODE_FLAG_INSTANCE;

                                parameter2->value_update = clone1;

                                list_destroy(response1);
                                return semantic_assignment(program, node1, clone1, major, flag);
                            }
                            else
                            {
                                semantic_error(program, type2, "Not a type, in confronting with (%s-%lld:%lld)",
                                    item1->position.path, item1->position.line, item1->position.column);
                                return -1;
                            }
                        }

                        if (cnt_response1 == 0)
                        {
                            semantic_error(program, type2, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    list_destroy(response1);
                }
                else
                {
                    list_t *response1 = list_create();
                    if (response1 == NULL)
                    {
                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r1 = semantic_expression(program, parameter2->value, response1, flag);
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
                            
                            int32_t r2 = semantic_assignment(program, node2, item1, node2, flag);
                            if (r2 == -1)
                            {
                                return -1;
                            }

                            list_destroy(response1);
                            return semantic_assignment(program, node1, item1, major, flag);
                        }

                        if (cnt_response1 == 0)
                        {
                            semantic_error(program, parameter2->value, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    list_destroy(response1);
                }
            }
        }
        else
        if (node2->kind == NODE_KIND_CLASS)
        {
            if ((node2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
            {
                semantic_error(program, major, "Not an instance of class, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                return -1;
            }

            if (property1->type != NULL)
            {
                node_t *type1 = (node_t *)property1->type;

                list_t *response1 = list_create();
                if (response1 == NULL)
                {
                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                        if (item1->kind == NODE_KIND_LAMBDA)
                        {
                            if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                            {
                                semantic_error(program, major, "Instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                return -1;
                            }

                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                        else
                        if (item1->kind == NODE_KIND_OBJECT)
                        {
                            if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                            {
                                semantic_error(program, major, "Instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            
                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                        else
                        if (item1->kind == NODE_KIND_CLASS)
                        {
                            if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                            {
                                semantic_error(program, major, "Instance of class, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r2 = semantic_tequality(program, item1, node2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 0)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                list_destroy(response1);
                                property1->value_update = node2;
                                return 1;
                            }
                        }
                        else
                        if (item1->kind == NODE_KIND_GENERIC)
                        {
                            if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                            {
                                semantic_error(program, major, "Instance of class, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            
                            node_generic_t *generic1 = (node_generic_t *)item1->value;
                            if (generic1->type != NULL)
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
                                            if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                            {
                                                semantic_error(program, item1, "Instance of class, in confronting with (%s-%lld:%lld)",
                                                    item2->position.path, item2->position.line, item2->position.column);
                                                return -1;
                                            }

                                            int32_t r3 = semantic_tequality(program, item2, node2);
                                            if (r3 == -1)
                                            {
                                                return -1;
                                            }
                                            else
                                            if (r3 == 0)
                                            {
                                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                return -1;
                                            }
                                            else
                                            if (r3 == 1)
                                            {
                                                list_destroy(response2);
                                                list_destroy(response1);
                                                property1->value_update = node2;
                                                return 1;
                                            }
                                        }
                                        else
                                        if (item2->kind == NODE_KIND_GENERIC)
                                        {
                                            if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                            {
                                                semantic_error(program, item1, "Instance of generic, in confronting with (%s-%lld:%lld)",
                                                    item2->position.path, item2->position.line, item2->position.column);
                                                return -1;
                                            }

                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        {
                                            semantic_error(program, item1, "Not a valid type, in confronting with (%s-%lld:%lld)",
                                                item2->position.path, item2->position.line, item2->position.column);
                                            return -1;
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
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                        else
                        {
                            semantic_error(program, type1, "Not a type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    if (cnt_response1 == 0)
                    {
                        semantic_error(program, type1, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                        return -1;
                    }
                }
            }
            else
            {
                if (property1->value_update != NULL)
                {
                    node_t *value_update1 = property1->value_update;
                    if (value_update1->kind == NODE_KIND_LAMBDA)
                    {
                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                        return -1;
                    }
                    else
                    if (value_update1->kind == NODE_KIND_OBJECT)
                    {
                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                        return -1;
                    }
                    else
                    {
                        int32_t r2 = semantic_tequality(program, value_update1, node2);
                        if (r2 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r2 == 0)
                        {
                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                        else
                        if (r2 == 1)
                        {
                            property1->value_update = node2;
                            return 1;
                        }
                    }
                }
                else
                {
                    list_t *response1 = list_create();
                    if (response1 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                            if (item1->kind == NODE_KIND_LAMBDA)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            if (item1->kind == NODE_KIND_OBJECT)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            {
                                int32_t r2 = semantic_tequality(program, item1, node2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    property1->value_update = node2;
                                    return 1;
                                }
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
            }
        }
        else
        if (node2->kind == NODE_KIND_GENERIC)
        {
            if ((node2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
            {
                semantic_error(program, major, "Not an instance of generic, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                return -1;
            }

            if (property1->type != NULL)
            {
                node_t *type1 = (node_t *)property1->type;

                list_t *response1 = list_create();
                if (response1 == NULL)
                {
                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                        if (item1->kind == NODE_KIND_LAMBDA)
                        {
                            if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                            {
                                semantic_error(program, major, "Instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                return -1;
                            }

                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                        else
                        if (item1->kind == NODE_KIND_OBJECT)
                        {
                            if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                            {
                                semantic_error(program, major, "Instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            
                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                        else
                        if (item1->kind == NODE_KIND_CLASS)
                        {
                            if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                            {
                                semantic_error(program, major, "Instance of class, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                return -1;
                            }

                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                        else
                        if (item1->kind == NODE_KIND_GENERIC)
                        {
                            if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                            {
                                semantic_error(program, major, "Instance of class, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            
                            node_generic_t *generic1 = (node_generic_t *)item1->value;
                            if (generic1->type != NULL)
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
                                            if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                            {
                                                semantic_error(program, item1, "Instance of class, in confronting with (%s-%lld:%lld)",
                                                    item2->position.path, item2->position.line, item2->position.column);
                                                return -1;
                                            }

                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        if (item2->kind == NODE_KIND_GENERIC)
                                        {
                                            if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                            {
                                                semantic_error(program, item1, "Instance of generic, in confronting with (%s-%lld:%lld)",
                                                    item2->position.path, item2->position.line, item2->position.column);
                                                return -1;
                                            }

                                            int32_t r3 = semantic_tequality(program, item2, node2);
                                            if (r3 == -1)
                                            {
                                                return -1;
                                            }
                                            else
                                            if (r3 == 0)
                                            {
                                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                return -1;
                                            }
                                            else
                                            if (r3 == 1)
                                            {
                                                list_destroy(response2);
                                                list_destroy(response1);
                                                property1->value_update = node2;
                                                return 1;
                                            }
                                        }
                                        else
                                        {
                                            semantic_error(program, item1, "Not a valid type, in confronting with (%s-%lld:%lld)",
                                                item2->position.path, item2->position.line, item2->position.column);
                                            return -1;
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
                                int32_t r3 = semantic_tequality(program, item1, node2);
                                if (r3 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r3 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (r3 == 1)
                                {
                                    list_destroy(response1);
                                    property1->value_update = node2;
                                    return 1;
                                }
                            }
                        }
                        else
                        {
                            semantic_error(program, type1, "Not a type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    if (cnt_response1 == 0)
                    {
                        semantic_error(program, type1, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                        return -1;
                    }
                }
            }
            else
            {
                if (property1->value_update != NULL)
                {
                    node_t *value_update1 = property1->value_update;
                    if (value_update1->kind == NODE_KIND_LAMBDA)
                    {
                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                        return -1;
                    }
                    else
                    if (value_update1->kind == NODE_KIND_OBJECT)
                    {
                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                        return -1;
                    }
                    else
                    {
                        int32_t r2 = semantic_tequality(program, value_update1, node2);
                        if (r2 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r2 == 0)
                        {
                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                        else
                        if (r2 == 1)
                        {
                            property1->value_update = node2;
                            return 1;
                        }
                    }
                }
                else
                {
                    list_t *response1 = list_create();
                    if (response1 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                            if (item1->kind == NODE_KIND_LAMBDA)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            if (item1->kind == NODE_KIND_OBJECT)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            {
                                int32_t r2 = semantic_tequality(program, item1, node2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    property1->value_update = node2;
                                    return 1;
                                }
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
            }
        }
        else
        if (node2->kind == NODE_KIND_LAMBDA)
        {
            node_lambda_t *fun2 = (node_lambda_t *)node2->value;
            if (property1->type != NULL)
            {
                node_t *type1 = property1->type;

                list_t *response1 = list_create();
                if (response1 == NULL)
                {
                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                        if (item1->kind == NODE_KIND_LAMBDA)
                        {
                            if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                            {
                                semantic_error(program, major, "Instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                return -1;
                            }

                            node_fn_t *fun1 = (node_fn_t *)item1->value;
                            int32_t r2 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 0)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            {
                                int32_t r3 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                if (r3 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r3 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                {
                                    list_destroy(response1);
                                    property1->value_update = node2;
                                    return 1;
                                }
                            }
                        }
                        else
                        {
                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    if (cnt_response1 == 0)
                    {
                        semantic_error(program, type1, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                        return -1;
                    }
                }
            }
            else
            {
                if (property1->value_update != NULL)
                {
                    node_t *value_update1 = property1->value_update;
                    if (value_update1->kind == NODE_KIND_LAMBDA)
                    {
                        node_lambda_t *fun1 = (node_lambda_t *)value_update1->value;
                        int32_t r1 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 0)
                        {
                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                        else
                        {
                            int32_t r2 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 0)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            {
                                property1->value_update = node2;
                                return 1;
                            }
                        }
                    }
                    else
                    {
                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                        return -1;
                    }
                }
                else
                {
                    list_t *response1 = list_create();
                    if (response1 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                            if (item1->kind == NODE_KIND_LAMBDA)
                            {
                                node_lambda_t *fun1 = (node_lambda_t *)item1->value;
                                int32_t r2 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                {
                                    int32_t r3 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                    if (r3 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r3 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        property1->value_update = node2;
                                        list_destroy(response1);
                                        return 1;
                                    }
                                }
                            }
                            else
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
            }
        }
        else
        if (node2->kind == NODE_KIND_OBJECT)
        {
            if ((node2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
            {
                semantic_error(program, major, "Not an instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                return -1;
            }

            if (property1->type != NULL)
            {
                node_t *type1 = property1->type;

                list_t *response1 = list_create();
                if (response1 == NULL)
                {
                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                        if (item1->kind == NODE_KIND_OBJECT)
                        {
                            if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                            {
                                semantic_error(program, major, "Instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r2 = semantic_eqaul_otos(program, item1, node2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 0)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            {
                                list_destroy(response1);
                                property1->value_update = node2;
                                return 1;
                            }
                        }
                        else
                        {
                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
            else
            {
                if (property1->value_update != NULL)
                {
                    node_t *value_update1 = property1->value_update;
                    if (value_update1->kind == NODE_KIND_OBJECT)
                    {
                        int32_t r1 = semantic_eqaul_osos(program, value_update1, node2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 0)
                        {
                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                        else
                        {
                            property1->value_update = node2;
                            return 1;
                        }
                    }
                    else
                    {
                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                        return -1;
                    }
                }
                else
                {
                    list_t *response1 = list_create();
                    if (response1 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                            if (item1->kind == NODE_KIND_OBJECT)
                            {
                                int32_t r1 = semantic_eqaul_osos(program, item1, node2);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                {
                                    property1->value_update = node2;
                                    return 1;
                                }
                            }
                            else
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
            }
        }
        else
        {
            semantic_error(program, major, "Not valid value, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
            return -1;
        }
    }
    else
    if (node1->kind == NODE_KIND_PARAMETER)
    {
        node_parameter_t *parameter1 = (node_parameter_t *)node1->value;

        if (node2->kind == NODE_KIND_VAR)
        {
            node_var_t *var2 = (node_var_t *)node2->value;
            if (var2->value_update != NULL)
            {
                node_t *value_update2 = var2->value_update;
                if (value_update2->kind == NODE_KIND_LAMBDA)
                {
                    if ((value_update2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                    {
                        semantic_error(program, major, "Not an instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            value_update2->position.path, value_update2->position.line, value_update2->position.column, __FILE__, __LINE__);
                        return -1;
                    }

                    node_lambda_t *fun2 = (node_lambda_t *)value_update2->value;
                    if (parameter1->type != NULL)
                    {
                        node_t *type1 = parameter1->type;

                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                                if (item1->kind == NODE_KIND_LAMBDA)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    node_fn_t *fun1 = (node_fn_t *)item1->value;
                                    int32_t r2 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        int32_t r3 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                        if (r3 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r3 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        {
                                            list_destroy(response1);
                                            parameter1->value_update = var2->value_update;
                                            return 1;
                                        }
                                    }
                                }
                                else
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                            }

                            if (cnt_response1 == 0)
                            {
                                semantic_error(program, type1, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                    else
                    {
                        if (parameter1->value_update != NULL)
                        {
                            node_t *value_update1 = parameter1->value_update;
                            if (value_update1->kind == NODE_KIND_LAMBDA)
                            {
                                node_lambda_t *fun1 = (node_lambda_t *)value_update1->value;
                                int32_t r1 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                {
                                    int32_t r2 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        parameter1->value_update = value_update2;
                                        return 1;
                                    }
                                }
                            }
                            else
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                        else
                        {
                            list_t *response1 = list_create();
                            if (response1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                                    if (item1->kind == NODE_KIND_LAMBDA)
                                    {
                                        node_lambda_t *fun1 = (node_lambda_t *)item1->value;
                                        int32_t r2 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        {
                                            int32_t r3 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                            if (r3 == -1)
                                            {
                                                return -1;
                                            }
                                            else
                                            if (r3 == 0)
                                            {
                                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                return -1;
                                            }
                                            else
                                            {
                                                parameter1->value_update = value_update2;
                                                list_destroy(response1);
                                                return 1;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
                    }
                }
                else
                if (value_update2->kind == NODE_KIND_OBJECT)
                {
                    if ((value_update2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                    {
                        semantic_error(program, major, "Not an instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            value_update2->position.path, value_update2->position.line, value_update2->position.column, __FILE__, __LINE__);
                        return -1;
                    }

                    if (parameter1->type != NULL)
                    {
                        node_t *type1 = parameter1->type;

                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                                if (item1->kind == NODE_KIND_OBJECT)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_eqaul_otos(program, item1, value_update2);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        list_destroy(response1);
                                        parameter1->value_update = var2->value_update;
                                        return 1;
                                    }
                                }
                                else
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
                    else
                    {
                        if (parameter1->value_update != NULL)
                        {
                            node_t *value_update1 = parameter1->value_update;
                            if (value_update1->kind == NODE_KIND_OBJECT)
                            {
                                int32_t r1 = semantic_eqaul_osos(program, value_update1, value_update2);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                {
                                    parameter1->value_update = var2->value_update;
                                    return 1;
                                }
                            }
                            else
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                        else
                        {
                            list_t *response1 = list_create();
                            if (response1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                                    if (item1->kind == NODE_KIND_OBJECT)
                                    {
                                        int32_t r1 = semantic_eqaul_osos(program, item1, value_update2);
                                        if (r1 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r1 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        {
                                            parameter1->value_update = var2->value_update;
                                            return 1;
                                        }
                                    }
                                    else
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
                    }
                }
                else
                {
                    if (parameter1->type != NULL)
                    {
                        node_t *type1 = (node_t *)parameter1->type;

                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                                if (item1->kind == NODE_KIND_LAMBDA)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (item1->kind == NODE_KIND_OBJECT)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (item1->kind == NODE_KIND_CLASS)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of class, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_tequality(program, item1, value_update2);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    if (r2 == 1)
                                    {
                                        if (value_update2->kind == NODE_KIND_CLASS)
                                        {
                                            list_destroy(response1);
                                            parameter1->value_update = value_update2;
                                            return 1;
                                        }
                                        else
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }
                                }
                                else
                                if (item1->kind == NODE_KIND_GENERIC)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of class, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    
                                    node_generic_t *generic1 = (node_generic_t *)item1->value;
                                    if (generic1->type != NULL)
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
                                                    if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                    {
                                                        semantic_error(program, item1, "Instance of class, in confronting with (%s-%lld:%lld)",
                                                            item2->position.path, item2->position.line, item2->position.column);
                                                        return -1;
                                                    }

                                                    int32_t r3 = semantic_tequality(program, item2, value_update2);
                                                    if (r3 == -1)
                                                    {
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 0)
                                                    {
                                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 1)
                                                    {
                                                        if (value_update2->kind == NODE_KIND_CLASS)
                                                        {
                                                            list_destroy(response2);
                                                            list_destroy(response1);
                                                            parameter1->value_update = value_update2;
                                                            return 1;
                                                        }
                                                        else
                                                        {
                                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                    }
                                                }
                                                else
                                                if (item2->kind == NODE_KIND_GENERIC)
                                                {
                                                    if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                    {
                                                        semantic_error(program, item1, "Instance of generic, in confronting with (%s-%lld:%lld)",
                                                            item2->position.path, item2->position.line, item2->position.column);
                                                        return -1;
                                                    }

                                                    int32_t r3 = semantic_tequality(program, item2, value_update2);
                                                    if (r3 == -1)
                                                    {
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 0)
                                                    {
                                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 1)
                                                    {
                                                        if (value_update2->kind == NODE_KIND_GENERIC)
                                                        {
                                                            list_destroy(response2);
                                                            list_destroy(response1);
                                                            parameter1->value_update = value_update2;
                                                            return 1;
                                                        }
                                                        else
                                                        {
                                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                    }
                                                }
                                                else
                                                {
                                                    semantic_error(program, item1, "Not a valid type, in confronting with (%s-%lld:%lld)",
                                                        item2->position.path, item2->position.line, item2->position.column);
                                                    return -1;
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
                                        int32_t r2 = semantic_tequality(program, item1, value_update2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            if (value_update2->kind == NODE_KIND_GENERIC)
                                            {
                                                list_destroy(response1);
                                                parameter1->value_update = value_update2;
                                                return 1;
                                            }
                                            else
                                            {
                                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    semantic_error(program, type1, "Not a type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
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
                    else
                    {
                        if (parameter1->value_update != NULL)
                        {
                            node_t *value_update1 = parameter1->value_update;
                            if (value_update1->kind == NODE_KIND_LAMBDA)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            if (value_update1->kind == NODE_KIND_OBJECT)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            {
                                int32_t r2 = semantic_tequality(program, value_update1, value_update2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    parameter1->value_update = value_update2;
                                    return 1;
                                }
                            }
                        }
                        else
                        {
                            list_t *response1 = list_create();
                            if (response1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                                    if (item1->kind == NODE_KIND_LAMBDA)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    if (item1->kind == NODE_KIND_OBJECT)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        int32_t r2 = semantic_tequality(program, item1, value_update2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            parameter1->value_update = value_update2;
                                            return 1;
                                        }
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
                    }
                }
            }
            else
            {
                semantic_error(program, var2->key, "Unitialized\n\tInternal:%s-%u", __FILE__, __LINE__);
                return -1;
            }
        }
        else
        if (node2->kind == NODE_KIND_ENTITY)
        {
            node_entity_t *entity2 = (node_entity_t *)node2->value;
            if (entity2->value_update != NULL)
            {
                node_t *value_update2 = entity2->value_update;
                if (value_update2->kind == NODE_KIND_LAMBDA)
                {
                    if ((value_update2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                    {
                        semantic_error(program, major, "Not an instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            value_update2->position.path, value_update2->position.line, value_update2->position.column, __FILE__, __LINE__);
                        return -1;
                    }

                    node_lambda_t *fun2 = (node_lambda_t *)value_update2->value;
                    if (parameter1->type != NULL)
                    {
                        node_t *type1 = parameter1->type;

                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                                if (item1->kind == NODE_KIND_LAMBDA)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    node_fn_t *fun1 = (node_fn_t *)item1->value;
                                    int32_t r2 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        int32_t r3 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                        if (r3 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r3 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        {
                                            list_destroy(response1);
                                            parameter1->value_update = entity2->value_update;
                                            return 1;
                                        }
                                    }
                                }
                                else
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                            }

                            if (cnt_response1 == 0)
                            {
                                semantic_error(program, type1, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                    else
                    {
                        if (parameter1->value_update != NULL)
                        {
                            node_t *value_update1 = parameter1->value_update;
                            if (value_update1->kind == NODE_KIND_LAMBDA)
                            {
                                node_lambda_t *fun1 = (node_lambda_t *)value_update1->value;
                                int32_t r1 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                {
                                    int32_t r2 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        parameter1->value_update = value_update2;
                                        return 1;
                                    }
                                }
                            }
                            else
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                        else
                        {
                            list_t *response1 = list_create();
                            if (response1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                                    if (item1->kind == NODE_KIND_LAMBDA)
                                    {
                                        node_lambda_t *fun1 = (node_lambda_t *)item1->value;
                                        int32_t r2 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        {
                                            int32_t r3 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                            if (r3 == -1)
                                            {
                                                return -1;
                                            }
                                            else
                                            if (r3 == 0)
                                            {
                                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                return -1;
                                            }
                                            else
                                            {
                                                parameter1->value_update = value_update2;
                                                list_destroy(response1);
                                                return 1;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
                    }
                }
                else
                if (value_update2->kind == NODE_KIND_OBJECT)
                {
                    if ((value_update2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                    {
                        semantic_error(program, major, "Not an instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            value_update2->position.path, value_update2->position.line, value_update2->position.column, __FILE__, __LINE__);
                        return -1;
                    }

                    if (parameter1->type != NULL)
                    {
                        node_t *type1 = parameter1->type;

                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                                if (item1->kind == NODE_KIND_OBJECT)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_eqaul_otos(program, item1, value_update2);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        list_destroy(response1);
                                        parameter1->value_update = entity2->value_update;
                                        return 1;
                                    }
                                }
                                else
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
                    else
                    {
                        if (parameter1->value_update != NULL)
                        {
                            node_t *value_update1 = parameter1->value_update;
                            if (value_update1->kind == NODE_KIND_OBJECT)
                            {
                                int32_t r1 = semantic_eqaul_osos(program, value_update1, value_update2);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                {
                                    parameter1->value_update = entity2->value_update;
                                    return 1;
                                }
                            }
                            else
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                        else
                        {
                            list_t *response1 = list_create();
                            if (response1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                                    if (item1->kind == NODE_KIND_OBJECT)
                                    {
                                        int32_t r1 = semantic_eqaul_osos(program, item1, value_update2);
                                        if (r1 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r1 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        {
                                            parameter1->value_update = entity2->value_update;
                                            return 1;
                                        }
                                    }
                                    else
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
                    }
                }
                else
                {
                    if (parameter1->type != NULL)
                    {
                        node_t *type1 = (node_t *)parameter1->type;

                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                                if (item1->kind == NODE_KIND_LAMBDA)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (item1->kind == NODE_KIND_OBJECT)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (item1->kind == NODE_KIND_CLASS)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of class, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_tequality(program, item1, value_update2);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    if (r2 == 1)
                                    {
                                        if (value_update2->kind == NODE_KIND_CLASS)
                                        {
                                            list_destroy(response1);
                                            parameter1->value_update = value_update2;
                                            return 1;
                                        }
                                        else
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }
                                }
                                else
                                if (item1->kind == NODE_KIND_GENERIC)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of class, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    
                                    node_generic_t *generic1 = (node_generic_t *)item1->value;
                                    if (generic1->type != NULL)
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
                                                    if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                    {
                                                        semantic_error(program, item1, "Instance of class, in confronting with (%s-%lld:%lld)",
                                                            item2->position.path, item2->position.line, item2->position.column);
                                                        return -1;
                                                    }

                                                    int32_t r3 = semantic_tequality(program, item2, value_update2);
                                                    if (r3 == -1)
                                                    {
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 0)
                                                    {
                                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 1)
                                                    {
                                                        if (value_update2->kind == NODE_KIND_CLASS)
                                                        {
                                                            list_destroy(response2);
                                                            list_destroy(response1);
                                                            parameter1->value_update = value_update2;
                                                            return 1;
                                                        }
                                                        else
                                                        {
                                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                    }
                                                }
                                                else
                                                if (item2->kind == NODE_KIND_GENERIC)
                                                {
                                                    if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                    {
                                                        semantic_error(program, item1, "Instance of generic, in confronting with (%s-%lld:%lld)",
                                                            item2->position.path, item2->position.line, item2->position.column);
                                                        return -1;
                                                    }

                                                    int32_t r3 = semantic_tequality(program, item2, value_update2);
                                                    if (r3 == -1)
                                                    {
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 0)
                                                    {
                                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 1)
                                                    {
                                                        if (value_update2->kind == NODE_KIND_GENERIC)
                                                        {
                                                            list_destroy(response2);
                                                            list_destroy(response1);
                                                            parameter1->value_update = value_update2;
                                                            return 1;
                                                        }
                                                        else
                                                        {
                                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                    }
                                                }
                                                else
                                                {
                                                    semantic_error(program, item1, "Not a valid type, in confronting with (%s-%lld:%lld)",
                                                        item2->position.path, item2->position.line, item2->position.column);
                                                    return -1;
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
                                        int32_t r2 = semantic_tequality(program, item1, value_update2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            if (value_update2->kind == NODE_KIND_GENERIC)
                                            {
                                                list_destroy(response1);
                                                parameter1->value_update = value_update2;
                                                return 1;
                                            }
                                            else
                                            {
                                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    semantic_error(program, type1, "Not a type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
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
                    else
                    {
                        if (parameter1->value_update != NULL)
                        {
                            node_t *value_update1 = parameter1->value_update;
                            if (value_update1->kind == NODE_KIND_LAMBDA)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            if (value_update1->kind == NODE_KIND_OBJECT)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            {
                                int32_t r2 = semantic_tequality(program, value_update1, value_update2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    parameter1->value_update = value_update2;
                                    return 1;
                                }
                            }
                        }
                        else
                        {
                            list_t *response1 = list_create();
                            if (response1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                                    if (item1->kind == NODE_KIND_LAMBDA)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    if (item1->kind == NODE_KIND_OBJECT)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        int32_t r2 = semantic_tequality(program, item1, value_update2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            parameter1->value_update = value_update2;
                                            return 1;
                                        }
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
                    }
                }
            }
            else
            {
                semantic_error(program, entity2->key, "Unitialized\n\tInternal:%s-%u", __FILE__, __LINE__);
                return -1;
            }
        }
        else
        if (node2->kind == NODE_KIND_PROPERTY)
        {
            node_property_t *property2 = (node_property_t *)node2->value;
            if (property2->value_update != NULL)
            {
                node_t *value_update2 = property2->value_update;
                if (value_update2->kind == NODE_KIND_LAMBDA)
                {
                    if ((value_update2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                    {
                        semantic_error(program, major, "Not an instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            value_update2->position.path, value_update2->position.line, value_update2->position.column, __FILE__, __LINE__);
                        return -1;
                    }

                    node_lambda_t *fun2 = (node_lambda_t *)value_update2->value;
                    if (parameter1->type != NULL)
                    {
                        node_t *type1 = parameter1->type;

                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                                if (item1->kind == NODE_KIND_LAMBDA)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    node_fn_t *fun1 = (node_fn_t *)item1->value;
                                    int32_t r2 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        int32_t r3 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                        if (r3 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r3 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        {
                                            list_destroy(response1);
                                            parameter1->value_update = property2->value_update;
                                            return 1;
                                        }
                                    }
                                }
                                else
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                            }

                            if (cnt_response1 == 0)
                            {
                                semantic_error(program, type1, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                    else
                    {
                        if (parameter1->value_update != NULL)
                        {
                            node_t *value_update1 = parameter1->value_update;
                            if (value_update1->kind == NODE_KIND_LAMBDA)
                            {
                                node_lambda_t *fun1 = (node_lambda_t *)value_update1->value;
                                int32_t r1 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                {
                                    int32_t r2 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        parameter1->value_update = value_update2;
                                        return 1;
                                    }
                                }
                            }
                            else
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                        else
                        {
                            list_t *response1 = list_create();
                            if (response1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                                    if (item1->kind == NODE_KIND_LAMBDA)
                                    {
                                        node_lambda_t *fun1 = (node_lambda_t *)item1->value;
                                        int32_t r2 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        {
                                            int32_t r3 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                            if (r3 == -1)
                                            {
                                                return -1;
                                            }
                                            else
                                            if (r3 == 0)
                                            {
                                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                return -1;
                                            }
                                            else
                                            {
                                                parameter1->value_update = value_update2;
                                                list_destroy(response1);
                                                return 1;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
                    }
                }
                else
                if (value_update2->kind == NODE_KIND_OBJECT)
                {
                    if ((value_update2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                    {
                        semantic_error(program, major, "Not an instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            value_update2->position.path, value_update2->position.line, value_update2->position.column, __FILE__, __LINE__);
                        return -1;
                    }

                    if (parameter1->type != NULL)
                    {
                        node_t *type1 = parameter1->type;

                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                                if (item1->kind == NODE_KIND_OBJECT)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_eqaul_otos(program, item1, value_update2);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        list_destroy(response1);
                                        parameter1->value_update = property2->value_update;
                                        return 1;
                                    }
                                }
                                else
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
                    else
                    {
                        if (parameter1->value_update != NULL)
                        {
                            node_t *value_update1 = parameter1->value_update;
                            if (value_update1->kind == NODE_KIND_OBJECT)
                            {
                                int32_t r1 = semantic_eqaul_osos(program, value_update1, value_update2);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                {
                                    parameter1->value_update = property2->value_update;
                                    return 1;
                                }
                            }
                            else
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                        else
                        {
                            list_t *response1 = list_create();
                            if (response1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                                    if (item1->kind == NODE_KIND_OBJECT)
                                    {
                                        int32_t r1 = semantic_eqaul_osos(program, item1, value_update2);
                                        if (r1 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r1 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        {
                                            parameter1->value_update = property2->value_update;
                                            return 1;
                                        }
                                    }
                                    else
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
                    }
                }
                else
                {
                    if (parameter1->type != NULL)
                    {
                        node_t *type1 = (node_t *)parameter1->type;

                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                                if (item1->kind == NODE_KIND_LAMBDA)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (item1->kind == NODE_KIND_OBJECT)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (item1->kind == NODE_KIND_CLASS)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of class, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_tequality(program, item1, value_update2);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    if (r2 == 1)
                                    {
                                        if (value_update2->kind == NODE_KIND_CLASS)
                                        {
                                            list_destroy(response1);
                                            parameter1->value_update = value_update2;
                                            return 1;
                                        }
                                        else
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }
                                }
                                else
                                if (item1->kind == NODE_KIND_GENERIC)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of class, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    
                                    node_generic_t *generic1 = (node_generic_t *)item1->value;
                                    if (generic1->type != NULL)
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
                                                    if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                    {
                                                        semantic_error(program, item1, "Instance of class, in confronting with (%s-%lld:%lld)",
                                                            item2->position.path, item2->position.line, item2->position.column);
                                                        return -1;
                                                    }

                                                    int32_t r3 = semantic_tequality(program, item2, value_update2);
                                                    if (r3 == -1)
                                                    {
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 0)
                                                    {
                                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 1)
                                                    {
                                                        if (value_update2->kind == NODE_KIND_CLASS)
                                                        {
                                                            list_destroy(response2);
                                                            list_destroy(response1);
                                                            parameter1->value_update = value_update2;
                                                            return 1;
                                                        }
                                                        else
                                                        {
                                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                    }
                                                }
                                                else
                                                if (item2->kind == NODE_KIND_GENERIC)
                                                {
                                                    if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                    {
                                                        semantic_error(program, item1, "Instance of generic, in confronting with (%s-%lld:%lld)",
                                                            item2->position.path, item2->position.line, item2->position.column);
                                                        return -1;
                                                    }

                                                    int32_t r3 = semantic_tequality(program, item2, value_update2);
                                                    if (r3 == -1)
                                                    {
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 0)
                                                    {
                                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 1)
                                                    {
                                                        if (value_update2->kind == NODE_KIND_GENERIC)
                                                        {
                                                            list_destroy(response2);
                                                            list_destroy(response1);
                                                            parameter1->value_update = value_update2;
                                                            return 1;
                                                        }
                                                        else
                                                        {
                                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                    }
                                                }
                                                else
                                                {
                                                    semantic_error(program, item1, "Not a valid type, in confronting with (%s-%lld:%lld)",
                                                        item2->position.path, item2->position.line, item2->position.column);
                                                    return -1;
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
                                        int32_t r2 = semantic_tequality(program, item1, value_update2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            if (value_update2->kind == NODE_KIND_GENERIC)
                                            {
                                                list_destroy(response1);
                                                parameter1->value_update = value_update2;
                                                return 1;
                                            }
                                            else
                                            {
                                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    semantic_error(program, type1, "Not a type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
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
                    else
                    {
                        if (parameter1->value_update != NULL)
                        {
                            node_t *value_update1 = parameter1->value_update;
                            if (value_update1->kind == NODE_KIND_LAMBDA)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            if (value_update1->kind == NODE_KIND_OBJECT)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            {
                                int32_t r2 = semantic_tequality(program, value_update1, value_update2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    parameter1->value_update = value_update2;
                                    return 1;
                                }
                            }
                        }
                        else
                        {
                            list_t *response1 = list_create();
                            if (response1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                                    if (item1->kind == NODE_KIND_LAMBDA)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    if (item1->kind == NODE_KIND_OBJECT)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        int32_t r2 = semantic_tequality(program, item1, value_update2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            parameter1->value_update = value_update2;
                                            return 1;
                                        }
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
                    }
                }
            }
            else
            {
                if (property2->type != NULL)
                {
                    node_t *type2 = property2->type;

                    list_t *response1 = list_create();
                    if (response1 == NULL)
                    {
                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r1 = semantic_resolve(program, type2->parent, type2, response1, flag);
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
                                if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    semantic_error(program, type2, "Instance of class, in confronting with (%s-%lld:%lld)",
                                        item1->position.path, item1->position.line, item1->position.column);
                                    return -1;
                                }

                                node_t *clone1 = node_clone(item1->parent, item1);
                                if (clone1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                clone1->flag |= NODE_FLAG_INSTANCE;

                                property2->value_update = clone1;

                                list_destroy(response1);
                                return semantic_assignment(program, node1, clone1, major, flag);
                            }
                            else
                            if (item1->kind == NODE_KIND_GENERIC)
                            {
                                if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    semantic_error(program, type2, "Instance of generic, in confronting with (%s-%lld:%lld)",
                                        item1->position.path, item1->position.line, item1->position.column);
                                    return -1;
                                }

                                node_generic_t *generic1 = (node_generic_t *)item1->value;
                                if (generic1->type != NULL)
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
                                                if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                {
                                                    semantic_error(program, item1, "Instance of class, in confronting with (%s-%lld:%lld)",
                                                        item2->position.path, item2->position.line, item2->position.column);
                                                    return -1;
                                                }

                                                node_t *clone1 = node_clone(item2->parent, item2);
                                                if (clone1 == NULL)
                                                {
                                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                clone1->flag |= NODE_FLAG_INSTANCE;

                                                property2->value_update = clone1;

                                                list_destroy(response1);
                                                return semantic_assignment(program, node1, clone1, major, flag);
                                            }
                                            else
                                            if (item2->kind == NODE_KIND_GENERIC)
                                            {
                                                if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                {
                                                    semantic_error(program, item1, "Instance of generic, in confronting with (%s-%lld:%lld)",
                                                        item2->position.path, item2->position.line, item2->position.column);
                                                    return -1;
                                                }

                                                node_t *clone1 = node_clone(item2->parent, item2);
                                                if (clone1 == NULL)
                                                {
                                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                clone1->flag |= NODE_FLAG_INSTANCE;

                                                property2->value_update = clone1;

                                                list_destroy(response1);
                                                return semantic_assignment(program, node1, clone1, major, flag);
                                            }
                                            else
                                            {
                                                semantic_error(program, item1, "Not a valid type, in confronting with (%s-%lld:%lld)",
                                                    item2->position.path, item2->position.line, item2->position.column);
                                                return -1;
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
                                    node_t *clone1 = node_clone(item1->parent, item1);
                                    if (clone1 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    clone1->flag |= NODE_FLAG_INSTANCE;

                                    property2->value_update = clone1;

                                    list_destroy(response1);
                                    return semantic_assignment(program, node1, clone1, major, flag);
                                }
                            }
                            else
                            if (item1->kind == NODE_KIND_LAMBDA)
                            {
                                if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    semantic_error(program, type2, "Instance of fun, in confronting with (%s-%lld:%lld)",
                                        item1->position.path, item1->position.line, item1->position.column);
                                    return -1;
                                }

                                node_t *clone1 = node_clone(item1->parent, item1);
                                if (clone1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                clone1->flag |= NODE_FLAG_INSTANCE;

                                property2->value_update = clone1;

                                list_destroy(response1);
                                return semantic_assignment(program, node1, clone1, major, flag);
                            }
                            else
                            if (item1->kind == NODE_KIND_OBJECT)
                            {
                                if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    semantic_error(program, type2, "Instance of object, in confronting with (%s-%lld:%lld)",
                                        item1->position.path, item1->position.line, item1->position.column);
                                    return -1;
                                }

                                node_t *clone1 = node_clone(item1->parent, item1);
                                if (clone1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                clone1->flag |= NODE_FLAG_INSTANCE;

                                property2->value_update = clone1;

                                list_destroy(response1);
                                return semantic_assignment(program, node1, clone1, major, flag);
                            }
                            else
                            {
                                semantic_error(program, type2, "Not a type, in confronting with (%s-%lld:%lld)",
                                    item1->position.path, item1->position.line, item1->position.column);
                                return -1;
                            }
                        }

                        if (cnt_response1 == 0)
                        {
                            semantic_error(program, type2, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    list_destroy(response1);
                }
                else
                {
                    list_t *response1 = list_create();
                    if (response1 == NULL)
                    {
                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r1 = semantic_expression(program, property2->value, response1, flag);
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
                            
                            int32_t r2 = semantic_assignment(program, node2, item1, node2, flag);
                            if (r2 == -1)
                            {
                                return -1;
                            }

                            list_destroy(response1);
                            return semantic_assignment(program, node1, item1, major, flag);
                        }

                        if (cnt_response1 == 0)
                        {
                            semantic_error(program, property2->value, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    list_destroy(response1);
                }
            }
        }
        else
        if (node2->kind == NODE_KIND_PARAMETER)
        {
            node_parameter_t *parameter2 = (node_parameter_t *)node2->value;
            if (parameter2->value_update != NULL)
            {
                node_t *value_update2 = parameter2->value_update;
                if (value_update2->kind == NODE_KIND_LAMBDA)
                {
                    if ((value_update2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                    {
                        semantic_error(program, major, "Not an instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            value_update2->position.path, value_update2->position.line, value_update2->position.column, __FILE__, __LINE__);
                        return -1;
                    }

                    node_lambda_t *fun2 = (node_lambda_t *)value_update2->value;
                    if (parameter1->type != NULL)
                    {
                        node_t *type1 = parameter1->type;

                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                                if (item1->kind == NODE_KIND_LAMBDA)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    node_fn_t *fun1 = (node_fn_t *)item1->value;
                                    int32_t r2 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        int32_t r3 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                        if (r3 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r3 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        {
                                            list_destroy(response1);
                                            parameter1->value_update = parameter2->value_update;
                                            return 1;
                                        }
                                    }
                                }
                                else
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                            }

                            if (cnt_response1 == 0)
                            {
                                semantic_error(program, type1, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                    else
                    {
                        if (parameter1->value_update != NULL)
                        {
                            node_t *value_update1 = parameter1->value_update;
                            if (value_update1->kind == NODE_KIND_LAMBDA)
                            {
                                node_lambda_t *fun1 = (node_lambda_t *)value_update1->value;
                                int32_t r1 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                {
                                    int32_t r2 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        parameter1->value_update = value_update2;
                                        return 1;
                                    }
                                }
                            }
                            else
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                        else
                        {
                            list_t *response1 = list_create();
                            if (response1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                                    if (item1->kind == NODE_KIND_LAMBDA)
                                    {
                                        node_lambda_t *fun1 = (node_lambda_t *)item1->value;
                                        int32_t r2 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        {
                                            int32_t r3 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                            if (r3 == -1)
                                            {
                                                return -1;
                                            }
                                            else
                                            if (r3 == 0)
                                            {
                                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                return -1;
                                            }
                                            else
                                            {
                                                parameter1->value_update = value_update2;
                                                list_destroy(response1);
                                                return 1;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
                    }
                }
                else
                if (value_update2->kind == NODE_KIND_OBJECT)
                {
                    if ((value_update2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                    {
                        semantic_error(program, major, "Not an instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            value_update2->position.path, value_update2->position.line, value_update2->position.column, __FILE__, __LINE__);
                        return -1;
                    }

                    if (parameter1->type != NULL)
                    {
                        node_t *type1 = parameter1->type;

                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                                if (item1->kind == NODE_KIND_OBJECT)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_eqaul_otos(program, item1, value_update2);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        list_destroy(response1);
                                        parameter1->value_update = parameter2->value_update;
                                        return 1;
                                    }
                                }
                                else
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
                    else
                    {
                        if (parameter1->value_update != NULL)
                        {
                            node_t *value_update1 = parameter1->value_update;
                            if (value_update1->kind == NODE_KIND_OBJECT)
                            {
                                int32_t r1 = semantic_eqaul_osos(program, value_update1, value_update2);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                {
                                    parameter1->value_update = parameter2->value_update;
                                    return 1;
                                }
                            }
                            else
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                        else
                        {
                            list_t *response1 = list_create();
                            if (response1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                                    if (item1->kind == NODE_KIND_OBJECT)
                                    {
                                        int32_t r1 = semantic_eqaul_osos(program, item1, value_update2);
                                        if (r1 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r1 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        {
                                            parameter1->value_update = parameter2->value_update;
                                            return 1;
                                        }
                                    }
                                    else
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
                    }
                }
                else
                {
                    if (parameter1->type != NULL)
                    {
                        node_t *type1 = (node_t *)parameter1->type;

                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                                if (item1->kind == NODE_KIND_LAMBDA)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (item1->kind == NODE_KIND_OBJECT)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (item1->kind == NODE_KIND_CLASS)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of class, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_tequality(program, item1, value_update2);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    if (r2 == 1)
                                    {
                                        if (value_update2->kind == NODE_KIND_CLASS)
                                        {
                                            list_destroy(response1);
                                            parameter1->value_update = value_update2;
                                            return 1;
                                        }
                                        else
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }
                                }
                                else
                                if (item1->kind == NODE_KIND_GENERIC)
                                {
                                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, major, "Instance of class, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    
                                    node_generic_t *generic1 = (node_generic_t *)item1->value;
                                    if (generic1->type != NULL)
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
                                                    if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                    {
                                                        semantic_error(program, item1, "Instance of class, in confronting with (%s-%lld:%lld)",
                                                            item2->position.path, item2->position.line, item2->position.column);
                                                        return -1;
                                                    }

                                                    int32_t r3 = semantic_tequality(program, item2, value_update2);
                                                    if (r3 == -1)
                                                    {
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 0)
                                                    {
                                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 1)
                                                    {
                                                        if (value_update2->kind == NODE_KIND_CLASS)
                                                        {
                                                            list_destroy(response2);
                                                            list_destroy(response1);
                                                            parameter1->value_update = value_update2;
                                                            return 1;
                                                        }
                                                        else
                                                        {
                                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                    }
                                                }
                                                else
                                                if (item2->kind == NODE_KIND_GENERIC)
                                                {
                                                    if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                    {
                                                        semantic_error(program, item1, "Instance of generic, in confronting with (%s-%lld:%lld)",
                                                            item2->position.path, item2->position.line, item2->position.column);
                                                        return -1;
                                                    }

                                                    int32_t r3 = semantic_tequality(program, item2, value_update2);
                                                    if (r3 == -1)
                                                    {
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 0)
                                                    {
                                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 1)
                                                    {
                                                        if (value_update2->kind == NODE_KIND_GENERIC)
                                                        {
                                                            list_destroy(response2);
                                                            list_destroy(response1);
                                                            parameter1->value_update = value_update2;
                                                            return 1;
                                                        }
                                                        else
                                                        {
                                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                    }
                                                }
                                                else
                                                {
                                                    semantic_error(program, item1, "Not a valid type, in confronting with (%s-%lld:%lld)",
                                                        item2->position.path, item2->position.line, item2->position.column);
                                                    return -1;
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
                                        int32_t r2 = semantic_tequality(program, item1, value_update2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            if (value_update2->kind == NODE_KIND_GENERIC)
                                            {
                                                list_destroy(response1);
                                                parameter1->value_update = value_update2;
                                                return 1;
                                            }
                                            else
                                            {
                                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    semantic_error(program, type1, "Not a type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
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
                    else
                    {
                        if (parameter1->value_update != NULL)
                        {
                            node_t *value_update1 = parameter1->value_update;
                            if (value_update1->kind == NODE_KIND_LAMBDA)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            if (value_update1->kind == NODE_KIND_OBJECT)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            {
                                int32_t r2 = semantic_tequality(program, value_update1, value_update2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    parameter1->value_update = value_update2;
                                    return 1;
                                }
                            }
                        }
                        else
                        {
                            list_t *response1 = list_create();
                            if (response1 == NULL)
                            {
                                fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                                    if (item1->kind == NODE_KIND_LAMBDA)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    if (item1->kind == NODE_KIND_OBJECT)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        int32_t r2 = semantic_tequality(program, item1, value_update2);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 0)
                                        {
                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            parameter1->value_update = value_update2;
                                            return 1;
                                        }
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
                    }
                }
            }
            else
            {
                if (parameter2->type != NULL)
                {
                    node_t *type2 = parameter2->type;

                    list_t *response1 = list_create();
                    if (response1 == NULL)
                    {
                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r1 = semantic_resolve(program, type2->parent, type2, response1, flag);
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
                                if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    semantic_error(program, type2, "Instance of class, in confronting with (%s-%lld:%lld)",
                                        item1->position.path, item1->position.line, item1->position.column);
                                    return -1;
                                }

                                node_t *clone1 = node_clone(item1->parent, item1);
                                if (clone1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                clone1->flag |= NODE_FLAG_INSTANCE;

                                parameter2->value_update = clone1;

                                list_destroy(response1);
                                return semantic_assignment(program, node1, clone1, major, flag);
                            }
                            else
                            if (item1->kind == NODE_KIND_GENERIC)
                            {
                                if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    semantic_error(program, type2, "Instance of generic, in confronting with (%s-%lld:%lld)",
                                        item1->position.path, item1->position.line, item1->position.column);
                                    return -1;
                                }

                                node_generic_t *generic1 = (node_generic_t *)item1->value;
                                if (generic1->type != NULL)
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
                                                if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                {
                                                    semantic_error(program, item1, "Instance of class, in confronting with (%s-%lld:%lld)",
                                                        item2->position.path, item2->position.line, item2->position.column);
                                                    return -1;
                                                }

                                                node_t *clone1 = node_clone(item2->parent, item2);
                                                if (clone1 == NULL)
                                                {
                                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                clone1->flag |= NODE_FLAG_INSTANCE;

                                                parameter2->value_update = clone1;

                                                list_destroy(response1);
                                                return semantic_assignment(program, node1, clone1, major, flag);
                                            }
                                            else
                                            if (item2->kind == NODE_KIND_GENERIC)
                                            {
                                                if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                {
                                                    semantic_error(program, item1, "Instance of generic, in confronting with (%s-%lld:%lld)",
                                                        item2->position.path, item2->position.line, item2->position.column);
                                                    return -1;
                                                }

                                                node_t *clone1 = node_clone(item2->parent, item2);
                                                if (clone1 == NULL)
                                                {
                                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                clone1->flag |= NODE_FLAG_INSTANCE;

                                                parameter2->value_update = clone1;

                                                list_destroy(response1);
                                                return semantic_assignment(program, node1, clone1, major, flag);
                                            }
                                            else
                                            {
                                                semantic_error(program, item1, "Not a valid type, in confronting with (%s-%lld:%lld)",
                                                    item2->position.path, item2->position.line, item2->position.column);
                                                return -1;
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
                                    node_t *clone1 = node_clone(item1->parent, item1);
                                    if (clone1 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    clone1->flag |= NODE_FLAG_INSTANCE;

                                    parameter2->value_update = clone1;

                                    list_destroy(response1);
                                    return semantic_assignment(program, node1, clone1, major, flag);
                                }
                            }
                            else
                            if (item1->kind == NODE_KIND_LAMBDA)
                            {
                                if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    semantic_error(program, type2, "Instance of fun, in confronting with (%s-%lld:%lld)",
                                        item1->position.path, item1->position.line, item1->position.column);
                                    return -1;
                                }

                                node_t *clone1 = node_clone(item1->parent, item1);
                                if (clone1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                clone1->flag |= NODE_FLAG_INSTANCE;

                                parameter2->value_update = clone1;

                                list_destroy(response1);
                                return semantic_assignment(program, node1, clone1, major, flag);
                            }
                            else
                            if (item1->kind == NODE_KIND_OBJECT)
                            {
                                if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    semantic_error(program, type2, "Instance of object, in confronting with (%s-%lld:%lld)",
                                        item1->position.path, item1->position.line, item1->position.column);
                                    return -1;
                                }

                                node_t *clone1 = node_clone(item1->parent, item1);
                                if (clone1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                clone1->flag |= NODE_FLAG_INSTANCE;

                                parameter2->value_update = clone1;

                                list_destroy(response1);
                                return semantic_assignment(program, node1, clone1, major, flag);
                            }
                            else
                            {
                                semantic_error(program, type2, "Not a type, in confronting with (%s-%lld:%lld)",
                                    item1->position.path, item1->position.line, item1->position.column);
                                return -1;
                            }
                        }

                        if (cnt_response1 == 0)
                        {
                            semantic_error(program, type2, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    list_destroy(response1);
                }
                else
                {
                    list_t *response1 = list_create();
                    if (response1 == NULL)
                    {
                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r1 = semantic_expression(program, parameter2->value, response1, flag);
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
                            
                            int32_t r2 = semantic_assignment(program, node2, item1, node2, flag);
                            if (r2 == -1)
                            {
                                return -1;
                            }

                            list_destroy(response1);
                            return semantic_assignment(program, node1, item1, major, flag);
                        }

                        if (cnt_response1 == 0)
                        {
                            semantic_error(program, parameter2->value, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    list_destroy(response1);
                }
            }
        }
        else
        if (node2->kind == NODE_KIND_CLASS)
        {
            if ((node2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
            {
                semantic_error(program, major, "Not an instance of class, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                return -1;
            }

            if (parameter1->type != NULL)
            {
                node_t *type1 = (node_t *)parameter1->type;

                list_t *response1 = list_create();
                if (response1 == NULL)
                {
                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                        if (item1->kind == NODE_KIND_LAMBDA)
                        {
                            if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                            {
                                semantic_error(program, major, "Instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                return -1;
                            }

                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                        else
                        if (item1->kind == NODE_KIND_OBJECT)
                        {
                            if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                            {
                                semantic_error(program, major, "Instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            
                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                        else
                        if (item1->kind == NODE_KIND_CLASS)
                        {
                            if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                            {
                                semantic_error(program, major, "Instance of class, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r2 = semantic_tequality(program, item1, node2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 0)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                list_destroy(response1);
                                parameter1->value_update = node2;
                                return 1;
                            }
                        }
                        else
                        if (item1->kind == NODE_KIND_GENERIC)
                        {
                            if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                            {
                                semantic_error(program, major, "Instance of class, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            
                            node_generic_t *generic1 = (node_generic_t *)item1->value;
                            if (generic1->type != NULL)
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
                                            if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                            {
                                                semantic_error(program, item1, "Instance of class, in confronting with (%s-%lld:%lld)",
                                                    item2->position.path, item2->position.line, item2->position.column);
                                                return -1;
                                            }

                                            int32_t r3 = semantic_tequality(program, item2, node2);
                                            if (r3 == -1)
                                            {
                                                return -1;
                                            }
                                            else
                                            if (r3 == 0)
                                            {
                                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                return -1;
                                            }
                                            else
                                            if (r3 == 1)
                                            {
                                                list_destroy(response2);
                                                list_destroy(response1);
                                                parameter1->value_update = node2;
                                                return 1;
                                            }
                                        }
                                        else
                                        if (item2->kind == NODE_KIND_GENERIC)
                                        {
                                            if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                            {
                                                semantic_error(program, item1, "Instance of generic, in confronting with (%s-%lld:%lld)",
                                                    item2->position.path, item2->position.line, item2->position.column);
                                                return -1;
                                            }

                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        {
                                            semantic_error(program, item1, "Not a valid type, in confronting with (%s-%lld:%lld)",
                                                item2->position.path, item2->position.line, item2->position.column);
                                            return -1;
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
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                        }
                        else
                        {
                            semantic_error(program, type1, "Not a type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    if (cnt_response1 == 0)
                    {
                        semantic_error(program, type1, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                        return -1;
                    }
                }
            }
            else
            {
                if (parameter1->value_update != NULL)
                {
                    node_t *value_update1 = parameter1->value_update;
                    if (value_update1->kind == NODE_KIND_LAMBDA)
                    {
                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                        return -1;
                    }
                    else
                    if (value_update1->kind == NODE_KIND_OBJECT)
                    {
                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                        return -1;
                    }
                    else
                    {
                        int32_t r2 = semantic_tequality(program, value_update1, node2);
                        if (r2 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r2 == 0)
                        {
                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                        else
                        if (r2 == 1)
                        {
                            parameter1->value_update = node2;
                            return 1;
                        }
                    }
                }
                else
                {
                    list_t *response1 = list_create();
                    if (response1 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                            if (item1->kind == NODE_KIND_LAMBDA)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            if (item1->kind == NODE_KIND_OBJECT)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            {
                                int32_t r2 = semantic_tequality(program, item1, node2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    parameter1->value_update = node2;
                                    return 1;
                                }
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
            }
        }
        else
        if (node2->kind == NODE_KIND_GENERIC)
        {
            if ((node2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
            {
                semantic_error(program, major, "Not an instance of generic, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                return -1;
            }

            if (parameter1->type != NULL)
            {
                node_t *type1 = (node_t *)parameter1->type;

                list_t *response1 = list_create();
                if (response1 == NULL)
                {
                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                        if (item1->kind == NODE_KIND_LAMBDA)
                        {
                            if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                            {
                                semantic_error(program, major, "Instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                return -1;
                            }

                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                        else
                        if (item1->kind == NODE_KIND_OBJECT)
                        {
                            if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                            {
                                semantic_error(program, major, "Instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            
                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                        else
                        if (item1->kind == NODE_KIND_CLASS)
                        {
                            if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                            {
                                semantic_error(program, major, "Instance of class, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                return -1;
                            }

                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                        else
                        if (item1->kind == NODE_KIND_GENERIC)
                        {
                            if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                            {
                                semantic_error(program, major, "Instance of class, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            
                            node_generic_t *generic1 = (node_generic_t *)item1->value;
                            if (generic1->type != NULL)
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
                                            if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                            {
                                                semantic_error(program, item1, "Instance of class, in confronting with (%s-%lld:%lld)",
                                                    item2->position.path, item2->position.line, item2->position.column);
                                                return -1;
                                            }

                                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                            return -1;
                                        }
                                        else
                                        if (item2->kind == NODE_KIND_GENERIC)
                                        {
                                            if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                            {
                                                semantic_error(program, item1, "Instance of generic, in confronting with (%s-%lld:%lld)",
                                                    item2->position.path, item2->position.line, item2->position.column);
                                                return -1;
                                            }

                                            int32_t r3 = semantic_tequality(program, item2, node2);
                                            if (r3 == -1)
                                            {
                                                return -1;
                                            }
                                            else
                                            if (r3 == 0)
                                            {
                                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                                return -1;
                                            }
                                            else
                                            if (r3 == 1)
                                            {
                                                list_destroy(response2);
                                                list_destroy(response1);
                                                parameter1->value_update = node2;
                                                return 1;
                                            }
                                        }
                                        else
                                        {
                                            semantic_error(program, item1, "Not a valid type, in confronting with (%s-%lld:%lld)",
                                                item2->position.path, item2->position.line, item2->position.column);
                                            return -1;
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
                                int32_t r3 = semantic_tequality(program, item1, node2);
                                if (r3 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r3 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (r3 == 1)
                                {
                                    list_destroy(response1);
                                    parameter1->value_update = node2;
                                    return 1;
                                }
                            }
                        }
                        else
                        {
                            semantic_error(program, type1, "Not a type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    if (cnt_response1 == 0)
                    {
                        semantic_error(program, type1, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                        return -1;
                    }
                }
            }
            else
            {
                if (parameter1->value_update != NULL)
                {
                    node_t *value_update1 = parameter1->value_update;
                    if (value_update1->kind == NODE_KIND_LAMBDA)
                    {
                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                        return -1;
                    }
                    else
                    if (value_update1->kind == NODE_KIND_OBJECT)
                    {
                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                        return -1;
                    }
                    else
                    {
                        int32_t r2 = semantic_tequality(program, value_update1, node2);
                        if (r2 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r2 == 0)
                        {
                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                        else
                        if (r2 == 1)
                        {
                            parameter1->value_update = node2;
                            return 1;
                        }
                    }
                }
                else
                {
                    list_t *response1 = list_create();
                    if (response1 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                            if (item1->kind == NODE_KIND_LAMBDA)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            if (item1->kind == NODE_KIND_OBJECT)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            {
                                int32_t r2 = semantic_tequality(program, item1, node2);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                if (r2 == 1)
                                {
                                    parameter1->value_update = node2;
                                    return 1;
                                }
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
            }
        }
        else
        if (node2->kind == NODE_KIND_LAMBDA)
        {
            node_lambda_t *fun2 = (node_lambda_t *)node2->value;
            if (parameter1->type != NULL)
            {
                node_t *type1 = parameter1->type;

                list_t *response1 = list_create();
                if (response1 == NULL)
                {
                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                        if (item1->kind == NODE_KIND_LAMBDA)
                        {
                            if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                            {
                                semantic_error(program, major, "Instance of fun, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                return -1;
                            }

                            node_fn_t *fun1 = (node_fn_t *)item1->value;
                            int32_t r2 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 0)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            {
                                int32_t r3 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                if (r3 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r3 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                {
                                    list_destroy(response1);
                                    parameter1->value_update = node2;
                                    return 1;
                                }
                            }
                        }
                        else
                        {
                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    if (cnt_response1 == 0)
                    {
                        semantic_error(program, type1, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                        return -1;
                    }
                }
            }
            else
            {
                if (parameter1->value_update != NULL)
                {
                    node_t *value_update1 = parameter1->value_update;
                    if (value_update1->kind == NODE_KIND_LAMBDA)
                    {
                        node_lambda_t *fun1 = (node_lambda_t *)value_update1->value;
                        int32_t r1 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 0)
                        {
                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                        else
                        {
                            int32_t r2 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 0)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            {
                                parameter1->value_update = node2;
                                return 1;
                            }
                        }
                    }
                    else
                    {
                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                        return -1;
                    }
                }
                else
                {
                    list_t *response1 = list_create();
                    if (response1 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                            if (item1->kind == NODE_KIND_LAMBDA)
                            {
                                node_lambda_t *fun1 = (node_lambda_t *)item1->value;
                                int32_t r2 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r2 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                {
                                    int32_t r3 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
                                    if (r3 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r3 == 0)
                                    {
                                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                        return -1;
                                    }
                                    else
                                    {
                                        parameter1->value_update = node2;
                                        list_destroy(response1);
                                        return 1;
                                    }
                                }
                            }
                            else
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
            }
        }
        else
        if (node2->kind == NODE_KIND_OBJECT)
        {
            if ((node2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
            {
                semantic_error(program, major, "Not an instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                return -1;
            }

            if (parameter1->type != NULL)
            {
                node_t *type1 = parameter1->type;

                list_t *response1 = list_create();
                if (response1 == NULL)
                {
                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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

                        if (item1->kind == NODE_KIND_OBJECT)
                        {
                            if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                            {
                                semantic_error(program, major, "Instance of object, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    item1->position.path, item1->position.line, item1->position.column, __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r2 = semantic_eqaul_otos(program, item1, node2);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 0)
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            {
                                list_destroy(response1);
                                parameter1->value_update = node2;
                                return 1;
                            }
                        }
                        else
                        {
                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
            else
            {
                if (parameter1->value_update != NULL)
                {
                    node_t *value_update1 = parameter1->value_update;
                    if (value_update1->kind == NODE_KIND_OBJECT)
                    {
                        int32_t r1 = semantic_eqaul_osos(program, value_update1, node2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 0)
                        {
                            semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                        else
                        {
                            parameter1->value_update = node2;
                            return 1;
                        }
                    }
                    else
                    {
                        semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                        return -1;
                    }
                }
                else
                {
                    list_t *response1 = list_create();
                    if (response1 == NULL)
                    {
                        fprintf(stderr, "%s-(%u):Unable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r1 = semantic_vresolve(program, node1, response1, flag);
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
                            if (item1->kind == NODE_KIND_OBJECT)
                            {
                                int32_t r1 = semantic_eqaul_osos(program, item1, node2);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 0)
                                {
                                    semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                                    return -1;
                                }
                                else
                                {
                                    parameter1->value_update = node2;
                                    return 1;
                                }
                            }
                            else
                            {
                                semantic_error(program, major, "Different type, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                    node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
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
            }
        }
        else
        {
            semantic_error(program, major, "Not valid value, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
            return -1;
        }
    }
    else
    if (node1->kind == NODE_KIND_ENTITY)
    {
        node_entity_t *entity1 = (node_entity_t *)node1->value;
        if (node2->kind == NODE_KIND_VAR)
        {
            node_var_t *var2 = (node_var_t *)node2->value;
            if (var2->value_update != NULL)
            {
                node_t *value_update2 = var2->value_update;
                if (value_update2->kind == NODE_KIND_LAMBDA)
                {
                    semantic_error(program, major, "It does not have a property, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                    return -1;
                }
                else
                if (value_update2->kind == NODE_KIND_OBJECT)
                {
                    if (entity1->type != NULL)
                    {
                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }

                        int32_t r1 = semantic_oselect(program, value_update2, entity1->type, response1, flag);
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
                                if (item1->kind == NODE_KIND_PAIR)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_vresolve(program, item1, response2, flag);
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
                                            entity1->value_update = item2;

                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }

                                        if (cnt_response2 == 0)
                                        {
                                            semantic_error(program, item1, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }

                                    list_destroy(response2);
                                }
                            }

                            if (cnt_response1 == 0)
                            {
                                if (entity1->value != NULL)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_vresolve(program, entity1->value, response2, flag);
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
                                            entity1->value_update = item2;

                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }

                                        if (cnt_response2 == 0)
                                        {
                                            semantic_error(program, entity1->value, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }

                                    list_destroy(response2);
                                }
                                else
                                {
                                    semantic_error(program, entity1->type, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
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
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }

                        int32_t r1 = semantic_oselect(program, value_update2, entity1->key, response1, flag);
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
                                if (item1->kind == NODE_KIND_PAIR)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_vresolve(program, item1, response2, flag);
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
                                            entity1->value_update = item2;

                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }

                                        if (cnt_response2 == 0)
                                        {
                                            semantic_error(program, item1, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }

                                    list_destroy(response2);
                                }
                            }

                            if (cnt_response1 == 0)
                            {
                                if (entity1->value != NULL)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_vresolve(program, entity1->value, response2, flag);
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
                                            entity1->value_update = item2;

                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }

                                        if (cnt_response2 == 0)
                                        {
                                            semantic_error(program, entity1->value, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }

                                    list_destroy(response2);
                                }
                                else
                                {
                                    semantic_error(program, entity1->key, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                    return -1;
                                }
                            }
                        }
                        list_destroy(response1);
                    }
                }
                else
                if (value_update2->kind == NODE_KIND_GENERIC)
                {
                    semantic_error(program, major, "It does not have a property, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                    return -1;
                }
                else
                {
                    if (entity1->type != NULL)
                    {
                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }

                        int32_t r1 = semantic_cselect(program, value_update2, entity1->type, response1, flag);
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
                                if (item1->kind == NODE_KIND_PROPERTY)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_vresolve(program, item1, response2, flag);
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
                                            entity1->value_update = item2;

                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }

                                        if (cnt_response2 == 0)
                                        {
                                            semantic_error(program, item1, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }

                                    list_destroy(response2);
                                }
                                
                            }

                            if (cnt_response1 == 0)
                            {
                                if (entity1->value != NULL)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_vresolve(program, entity1->value, response2, flag);
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
                                            entity1->value_update = item2;

                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }

                                        if (cnt_response2 == 0)
                                        {
                                            semantic_error(program, entity1->value, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }

                                    list_destroy(response2);
                                }
                                else
                                {
                                    semantic_error(program, entity1->type, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
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
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }

                        int32_t r1 = semantic_cselect(program, value_update2, entity1->key, response1, flag);
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
                                if (item1->kind == NODE_KIND_PROPERTY)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_vresolve(program, item1, response2, flag);
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
                                            entity1->value_update = item2;

                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }

                                        if (cnt_response2 == 0)
                                        {
                                            semantic_error(program, item1, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }

                                    list_destroy(response2);
                                }
                                
                            }

                            if (cnt_response1 == 0)
                            {
                                if (entity1->value != NULL)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_vresolve(program, entity1->value, response2, flag);
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
                                            entity1->value_update = item2;
                                            
                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }

                                        if (cnt_response2 == 0)
                                        {
                                            semantic_error(program, entity1->value, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }

                                    list_destroy(response2);
                                }
                                else
                                {
                                    semantic_error(program, entity1->key, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                    return -1;
                                }
                            }
                        }

                        list_destroy(response1);
                    }
                }
            }
            else
            {
                semantic_error(program, var2->key, "Unitialized\n\tInternal:%s-%u", __FILE__, __LINE__);
                return -1;
            }
        }
        else
        if (node2->kind == NODE_KIND_ENTITY)
        {
            node_entity_t *entity2 = (node_entity_t *)node2->value;
            if (entity2->value_update != NULL)
            {
                node_t *value_update2 = entity2->value_update;
                if (value_update2->kind == NODE_KIND_LAMBDA)
                {
                    semantic_error(program, major, "It does not have a property, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                    return -1;
                }
                else
                if (value_update2->kind == NODE_KIND_OBJECT)
                {
                    if (entity1->type != NULL)
                    {
                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }

                        int32_t r1 = semantic_oselect(program, value_update2, entity1->type, response1, flag);
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
                                if (item1->kind == NODE_KIND_PAIR)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_vresolve(program, item1, response2, flag);
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
                                            entity1->value_update = item2;

                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }

                                        if (cnt_response2 == 0)
                                        {
                                            semantic_error(program, item1, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }

                                    list_destroy(response2);
                                }
                            }

                            if (cnt_response1 == 0)
                            {
                                if (entity1->value != NULL)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_vresolve(program, entity1->value, response2, flag);
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
                                            entity1->value_update = item2;

                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }

                                        if (cnt_response2 == 0)
                                        {
                                            semantic_error(program, entity1->value, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }

                                    list_destroy(response2);
                                }
                                else
                                {
                                    semantic_error(program, entity1->type, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
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
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }

                        int32_t r1 = semantic_oselect(program, value_update2, entity1->key, response1, flag);
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
                                if (item1->kind == NODE_KIND_PAIR)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_vresolve(program, item1, response2, flag);
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
                                            entity1->value_update = item2;

                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }

                                        if (cnt_response2 == 0)
                                        {
                                            semantic_error(program, item1, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }

                                    list_destroy(response2);
                                }
                            }

                            if (cnt_response1 == 0)
                            {
                                if (entity1->value != NULL)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_vresolve(program, entity1->value, response2, flag);
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
                                            entity1->value_update = item2;

                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }

                                        if (cnt_response2 == 0)
                                        {
                                            semantic_error(program, entity1->value, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }

                                    list_destroy(response2);
                                }
                                else
                                {
                                    semantic_error(program, entity1->key, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                    return -1;
                                }
                            }
                        }
                        list_destroy(response1);
                    }
                }
                else
                if (value_update2->kind == NODE_KIND_GENERIC)
                {
                    semantic_error(program, major, "It does not have a property, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                    return -1;
                }
                else
                {
                    if (entity1->type != NULL)
                    {
                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }

                        int32_t r1 = semantic_cselect(program, value_update2, entity1->type, response1, flag);
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
                                if (item1->kind == NODE_KIND_PROPERTY)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_vresolve(program, item1, response2, flag);
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
                                            entity1->value_update = item2;

                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }

                                        if (cnt_response2 == 0)
                                        {
                                            semantic_error(program, item1, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }

                                    list_destroy(response2);
                                }
                                
                            }

                            if (cnt_response1 == 0)
                            {
                                if (entity1->value != NULL)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_vresolve(program, entity1->value, response2, flag);
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
                                            entity1->value_update = item2;

                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }

                                        if (cnt_response2 == 0)
                                        {
                                            semantic_error(program, entity1->value, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }

                                    list_destroy(response2);
                                }
                                else
                                {
                                    semantic_error(program, entity1->type, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
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
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }

                        int32_t r1 = semantic_cselect(program, value_update2, entity1->key, response1, flag);
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
                                if (item1->kind == NODE_KIND_PROPERTY)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_vresolve(program, item1, response2, flag);
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
                                            entity1->value_update = item2;

                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }

                                        if (cnt_response2 == 0)
                                        {
                                            semantic_error(program, item1, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }

                                    list_destroy(response2);
                                }
                                
                            }

                            if (cnt_response1 == 0)
                            {
                                if (entity1->value != NULL)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_vresolve(program, entity1->value, response2, flag);
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
                                            entity1->value_update = item2;
                                            
                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }

                                        if (cnt_response2 == 0)
                                        {
                                            semantic_error(program, entity1->value, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }

                                    list_destroy(response2);
                                }
                                else
                                {
                                    semantic_error(program, entity1->key, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                    return -1;
                                }
                            }
                        }

                        list_destroy(response1);
                    }
                }
            }
            else
            {
                semantic_error(program, entity2->key, "Unitialized\n\tInternal:%s-%u", __FILE__, __LINE__);
                return -1;
            }
        }
        else
        if (node2->kind == NODE_KIND_PROPERTY)
        {
            node_property_t *property2 = (node_property_t *)node2->value;
            if (property2->value_update != NULL)
            {
                node_t *value_update2 = property2->value_update;
                if (value_update2->kind == NODE_KIND_LAMBDA)
                {
                    semantic_error(program, major, "It does not have a property, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                    return -1;
                }
                else
                if (value_update2->kind == NODE_KIND_OBJECT)
                {
                    if (entity1->type != NULL)
                    {
                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }

                        int32_t r1 = semantic_oselect(program, value_update2, entity1->type, response1, flag);
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
                                if (item1->kind == NODE_KIND_PAIR)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_vresolve(program, item1, response2, flag);
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
                                            entity1->value_update = item2;

                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }

                                        if (cnt_response2 == 0)
                                        {
                                            semantic_error(program, item1, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }

                                    list_destroy(response2);
                                }
                            }

                            if (cnt_response1 == 0)
                            {
                                if (entity1->value != NULL)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_vresolve(program, entity1->value, response2, flag);
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
                                            entity1->value_update = item2;

                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }

                                        if (cnt_response2 == 0)
                                        {
                                            semantic_error(program, entity1->value, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }

                                    list_destroy(response2);
                                }
                                else
                                {
                                    semantic_error(program, entity1->type, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
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
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }

                        int32_t r1 = semantic_oselect(program, value_update2, entity1->key, response1, flag);
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
                                if (item1->kind == NODE_KIND_PAIR)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_vresolve(program, item1, response2, flag);
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
                                            entity1->value_update = item2;

                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }

                                        if (cnt_response2 == 0)
                                        {
                                            semantic_error(program, item1, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }

                                    list_destroy(response2);
                                }
                            }

                            if (cnt_response1 == 0)
                            {
                                if (entity1->value != NULL)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_vresolve(program, entity1->value, response2, flag);
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
                                            entity1->value_update = item2;

                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }

                                        if (cnt_response2 == 0)
                                        {
                                            semantic_error(program, entity1->value, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }

                                    list_destroy(response2);
                                }
                                else
                                {
                                    semantic_error(program, entity1->key, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                    return -1;
                                }
                            }
                        }
                        list_destroy(response1);
                    }
                }
                else
                if (value_update2->kind == NODE_KIND_GENERIC)
                {
                    semantic_error(program, major, "It does not have a property, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                    return -1;
                }
                else
                {
                    if (entity1->type != NULL)
                    {
                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }

                        int32_t r1 = semantic_cselect(program, value_update2, entity1->type, response1, flag);
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
                                if (item1->kind == NODE_KIND_PROPERTY)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_vresolve(program, item1, response2, flag);
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
                                            entity1->value_update = item2;

                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }

                                        if (cnt_response2 == 0)
                                        {
                                            semantic_error(program, item1, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }

                                    list_destroy(response2);
                                }
                                
                            }

                            if (cnt_response1 == 0)
                            {
                                if (entity1->value != NULL)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_vresolve(program, entity1->value, response2, flag);
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
                                            entity1->value_update = item2;

                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }

                                        if (cnt_response2 == 0)
                                        {
                                            semantic_error(program, entity1->value, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }

                                    list_destroy(response2);
                                }
                                else
                                {
                                    semantic_error(program, entity1->type, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
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
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }

                        int32_t r1 = semantic_cselect(program, value_update2, entity1->key, response1, flag);
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
                                if (item1->kind == NODE_KIND_PROPERTY)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_vresolve(program, item1, response2, flag);
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
                                            entity1->value_update = item2;

                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }

                                        if (cnt_response2 == 0)
                                        {
                                            semantic_error(program, item1, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }

                                    list_destroy(response2);
                                }
                                
                            }

                            if (cnt_response1 == 0)
                            {
                                if (entity1->value != NULL)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_vresolve(program, entity1->value, response2, flag);
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
                                            entity1->value_update = item2;
                                            
                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }

                                        if (cnt_response2 == 0)
                                        {
                                            semantic_error(program, entity1->value, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }

                                    list_destroy(response2);
                                }
                                else
                                {
                                    semantic_error(program, entity1->key, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                    return -1;
                                }
                            }
                        }

                        list_destroy(response1);
                    }
                }
            }
            else
            {
                if (property2->type != NULL)
                {
                    node_t *type2 = property2->type;

                    list_t *response1 = list_create();
                    if (response1 == NULL)
                    {
                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r1 = semantic_resolve(program, type2->parent, type2, response1, flag);
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
                                if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    semantic_error(program, type2, "Instance of class, in confronting with (%s-%lld:%lld)",
                                        item1->position.path, item1->position.line, item1->position.column);
                                    return -1;
                                }

                                node_t *clone1 = node_clone(item1->parent, item1);
                                if (clone1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                clone1->flag |= NODE_FLAG_INSTANCE;

                                property2->value_update = clone1;

                                list_destroy(response1);
                                return semantic_assignment(program, node1, clone1, major, flag);
                            }
                            else
                            if (item1->kind == NODE_KIND_GENERIC)
                            {
                                if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    semantic_error(program, type2, "Instance of generic, in confronting with (%s-%lld:%lld)",
                                        item1->position.path, item1->position.line, item1->position.column);
                                    return -1;
                                }

                                node_generic_t *generic1 = (node_generic_t *)item1->value;
                                if (generic1->type != NULL)
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
                                                if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                {
                                                    semantic_error(program, item1, "Instance of class, in confronting with (%s-%lld:%lld)",
                                                        item2->position.path, item2->position.line, item2->position.column);
                                                    return -1;
                                                }

                                                node_t *clone1 = node_clone(item2->parent, item2);
                                                if (clone1 == NULL)
                                                {
                                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                clone1->flag |= NODE_FLAG_INSTANCE;

                                                property2->value_update = clone1;

                                                list_destroy(response1);
                                                return semantic_assignment(program, node1, clone1, major, flag);
                                            }
                                            else
                                            if (item2->kind == NODE_KIND_GENERIC)
                                            {
                                                if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                {
                                                    semantic_error(program, item1, "Instance of generic, in confronting with (%s-%lld:%lld)",
                                                        item2->position.path, item2->position.line, item2->position.column);
                                                    return -1;
                                                }

                                                node_t *clone1 = node_clone(item2->parent, item2);
                                                if (clone1 == NULL)
                                                {
                                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                clone1->flag |= NODE_FLAG_INSTANCE;

                                                property2->value_update = clone1;

                                                list_destroy(response1);
                                                return semantic_assignment(program, node1, clone1, major, flag);
                                            }
                                            else
                                            {
                                                semantic_error(program, item1, "Not a valid type, in confronting with (%s-%lld:%lld)",
                                                    item2->position.path, item2->position.line, item2->position.column);
                                                return -1;
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
                                    node_t *clone1 = node_clone(item1->parent, item1);
                                    if (clone1 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    clone1->flag |= NODE_FLAG_INSTANCE;

                                    property2->value_update = clone1;

                                    list_destroy(response1);
                                    return semantic_assignment(program, node1, clone1, major, flag);
                                }
                            }
                            else
                            if (item1->kind == NODE_KIND_LAMBDA)
                            {
                                if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    semantic_error(program, type2, "Instance of fun, in confronting with (%s-%lld:%lld)",
                                        item1->position.path, item1->position.line, item1->position.column);
                                    return -1;
                                }

                                node_t *clone1 = node_clone(item1->parent, item1);
                                if (clone1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                clone1->flag |= NODE_FLAG_INSTANCE;

                                property2->value_update = clone1;

                                list_destroy(response1);
                                return semantic_assignment(program, node1, clone1, major, flag);
                            }
                            else
                            if (item1->kind == NODE_KIND_OBJECT)
                            {
                                if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    semantic_error(program, type2, "Instance of object, in confronting with (%s-%lld:%lld)",
                                        item1->position.path, item1->position.line, item1->position.column);
                                    return -1;
                                }

                                node_t *clone1 = node_clone(item1->parent, item1);
                                if (clone1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                clone1->flag |= NODE_FLAG_INSTANCE;

                                property2->value_update = clone1;

                                list_destroy(response1);
                                return semantic_assignment(program, node1, clone1, major, flag);
                            }
                            else
                            {
                                semantic_error(program, type2, "Not a type, in confronting with (%s-%lld:%lld)",
                                    item1->position.path, item1->position.line, item1->position.column);
                                return -1;
                            }
                        }

                        if (cnt_response1 == 0)
                        {
                            semantic_error(program, type2, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    list_destroy(response1);
                }
                else
                {
                    list_t *response1 = list_create();
                    if (response1 == NULL)
                    {
                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r1 = semantic_expression(program, property2->value, response1, flag);
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
                            
                            int32_t r2 = semantic_assignment(program, node2, item1, node2, flag);
                            if (r2 == -1)
                            {
                                return -1;
                            }

                            list_destroy(response1);
                            return semantic_assignment(program, node1, item1, major, flag);
                        }

                        if (cnt_response1 == 0)
                        {
                            semantic_error(program, property2->value, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    list_destroy(response1);
                }
            }
        }
        else
        if (node2->kind == NODE_KIND_PARAMETER)
        {
            node_parameter_t *parameter2 = (node_parameter_t *)node2->value;
            if (parameter2->value_update != NULL)
            {
                node_t *value_update2 = parameter2->value_update;
                if (value_update2->kind == NODE_KIND_LAMBDA)
                {
                    semantic_error(program, major, "It does not have a property, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                    return -1;
                }
                else
                if (value_update2->kind == NODE_KIND_OBJECT)
                {
                    if (entity1->type != NULL)
                    {
                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }

                        int32_t r1 = semantic_oselect(program, value_update2, entity1->type, response1, flag);
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
                                if (item1->kind == NODE_KIND_PAIR)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_vresolve(program, item1, response2, flag);
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
                                            entity1->value_update = item2;

                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }

                                        if (cnt_response2 == 0)
                                        {
                                            semantic_error(program, item1, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }

                                    list_destroy(response2);
                                }
                            }

                            if (cnt_response1 == 0)
                            {
                                if (entity1->value != NULL)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_vresolve(program, entity1->value, response2, flag);
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
                                            entity1->value_update = item2;

                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }

                                        if (cnt_response2 == 0)
                                        {
                                            semantic_error(program, entity1->value, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }

                                    list_destroy(response2);
                                }
                                else
                                {
                                    semantic_error(program, entity1->type, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
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
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }

                        int32_t r1 = semantic_oselect(program, value_update2, entity1->key, response1, flag);
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
                                if (item1->kind == NODE_KIND_PAIR)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_vresolve(program, item1, response2, flag);
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
                                            entity1->value_update = item2;

                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }

                                        if (cnt_response2 == 0)
                                        {
                                            semantic_error(program, item1, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }

                                    list_destroy(response2);
                                }
                            }

                            if (cnt_response1 == 0)
                            {
                                if (entity1->value != NULL)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_vresolve(program, entity1->value, response2, flag);
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
                                            entity1->value_update = item2;

                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }

                                        if (cnt_response2 == 0)
                                        {
                                            semantic_error(program, entity1->value, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }

                                    list_destroy(response2);
                                }
                                else
                                {
                                    semantic_error(program, entity1->key, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                    return -1;
                                }
                            }
                        }
                        list_destroy(response1);
                    }
                }
                else
                if (value_update2->kind == NODE_KIND_GENERIC)
                {
                    semantic_error(program, major, "It does not have a property, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                        node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
                    return -1;
                }
                else
                {
                    if (entity1->type != NULL)
                    {
                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }

                        int32_t r1 = semantic_cselect(program, value_update2, entity1->type, response1, flag);
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
                                if (item1->kind == NODE_KIND_PROPERTY)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_vresolve(program, item1, response2, flag);
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
                                            entity1->value_update = item2;

                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }

                                        if (cnt_response2 == 0)
                                        {
                                            semantic_error(program, item1, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }

                                    list_destroy(response2);
                                }
                                
                            }

                            if (cnt_response1 == 0)
                            {
                                if (entity1->value != NULL)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_vresolve(program, entity1->value, response2, flag);
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
                                            entity1->value_update = item2;

                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }

                                        if (cnt_response2 == 0)
                                        {
                                            semantic_error(program, entity1->value, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }

                                    list_destroy(response2);
                                }
                                else
                                {
                                    semantic_error(program, entity1->type, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
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
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }

                        int32_t r1 = semantic_cselect(program, value_update2, entity1->key, response1, flag);
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
                                if (item1->kind == NODE_KIND_PROPERTY)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_vresolve(program, item1, response2, flag);
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
                                            entity1->value_update = item2;

                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }

                                        if (cnt_response2 == 0)
                                        {
                                            semantic_error(program, item1, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }

                                    list_destroy(response2);
                                }
                                
                            }

                            if (cnt_response1 == 0)
                            {
                                if (entity1->value != NULL)
                                {
                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r2 = semantic_vresolve(program, entity1->value, response2, flag);
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
                                            entity1->value_update = item2;
                                            
                                            list_destroy(response2);
                                            list_destroy(response1);
                                            return 1;
                                        }

                                        if (cnt_response2 == 0)
                                        {
                                            semantic_error(program, entity1->value, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }

                                    list_destroy(response2);
                                }
                                else
                                {
                                    semantic_error(program, entity1->key, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                    return -1;
                                }
                            }
                        }

                        list_destroy(response1);
                    }
                }
            }
            else
            {
                if (parameter2->type != NULL)
                {
                    node_t *type2 = parameter2->type;

                    list_t *response1 = list_create();
                    if (response1 == NULL)
                    {
                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r1 = semantic_resolve(program, type2->parent, type2, response1, flag);
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
                                if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    semantic_error(program, type2, "Instance of class, in confronting with (%s-%lld:%lld)",
                                        item1->position.path, item1->position.line, item1->position.column);
                                    return -1;
                                }

                                node_t *clone1 = node_clone(item1->parent, item1);
                                if (clone1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                clone1->flag |= NODE_FLAG_INSTANCE;

                                parameter2->value_update = clone1;

                                list_destroy(response1);
                                return semantic_assignment(program, node1, clone1, major, flag);
                            }
                            else
                            if (item1->kind == NODE_KIND_GENERIC)
                            {
                                if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    semantic_error(program, type2, "Instance of generic, in confronting with (%s-%lld:%lld)",
                                        item1->position.path, item1->position.line, item1->position.column);
                                    return -1;
                                }

                                node_generic_t *generic1 = (node_generic_t *)item1->value;
                                if (generic1->type != NULL)
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
                                                if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                {
                                                    semantic_error(program, item1, "Instance of class, in confronting with (%s-%lld:%lld)",
                                                        item2->position.path, item2->position.line, item2->position.column);
                                                    return -1;
                                                }

                                                node_t *clone1 = node_clone(item2->parent, item2);
                                                if (clone1 == NULL)
                                                {
                                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                clone1->flag |= NODE_FLAG_INSTANCE;

                                                parameter2->value_update = clone1;

                                                list_destroy(response1);
                                                return semantic_assignment(program, node1, clone1, major, flag);
                                            }
                                            else
                                            if (item2->kind == NODE_KIND_GENERIC)
                                            {
                                                if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                {
                                                    semantic_error(program, item1, "Instance of generic, in confronting with (%s-%lld:%lld)",
                                                        item2->position.path, item2->position.line, item2->position.column);
                                                    return -1;
                                                }

                                                node_t *clone1 = node_clone(item2->parent, item2);
                                                if (clone1 == NULL)
                                                {
                                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                clone1->flag |= NODE_FLAG_INSTANCE;

                                                parameter2->value_update = clone1;

                                                list_destroy(response1);
                                                return semantic_assignment(program, node1, clone1, major, flag);
                                            }
                                            else
                                            {
                                                semantic_error(program, item1, "Not a valid type, in confronting with (%s-%lld:%lld)",
                                                    item2->position.path, item2->position.line, item2->position.column);
                                                return -1;
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
                                    node_t *clone1 = node_clone(item1->parent, item1);
                                    if (clone1 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    clone1->flag |= NODE_FLAG_INSTANCE;

                                    parameter2->value_update = clone1;

                                    list_destroy(response1);
                                    return semantic_assignment(program, node1, clone1, major, flag);
                                }
                            }
                            else
                            if (item1->kind == NODE_KIND_LAMBDA)
                            {
                                if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    semantic_error(program, type2, "Instance of fun, in confronting with (%s-%lld:%lld)",
                                        item1->position.path, item1->position.line, item1->position.column);
                                    return -1;
                                }

                                node_t *clone1 = node_clone(item1->parent, item1);
                                if (clone1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                clone1->flag |= NODE_FLAG_INSTANCE;

                                parameter2->value_update = clone1;

                                list_destroy(response1);
                                return semantic_assignment(program, node1, clone1, major, flag);
                            }
                            else
                            if (item1->kind == NODE_KIND_OBJECT)
                            {
                                if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    semantic_error(program, type2, "Instance of object, in confronting with (%s-%lld:%lld)",
                                        item1->position.path, item1->position.line, item1->position.column);
                                    return -1;
                                }

                                node_t *clone1 = node_clone(item1->parent, item1);
                                if (clone1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                clone1->flag |= NODE_FLAG_INSTANCE;

                                parameter2->value_update = clone1;

                                list_destroy(response1);
                                return semantic_assignment(program, node1, clone1, major, flag);
                            }
                            else
                            {
                                semantic_error(program, type2, "Not a type, in confronting with (%s-%lld:%lld)",
                                    item1->position.path, item1->position.line, item1->position.column);
                                return -1;
                            }
                        }

                        if (cnt_response1 == 0)
                        {
                            semantic_error(program, type2, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    list_destroy(response1);
                }
                else
                {
                    list_t *response1 = list_create();
                    if (response1 == NULL)
                    {
                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r1 = semantic_expression(program, parameter2->value, response1, flag);
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
                            
                            int32_t r2 = semantic_assignment(program, node2, item1, node2, flag);
                            if (r2 == -1)
                            {
                                return -1;
                            }

                            list_destroy(response1);
                            return semantic_assignment(program, node1, item1, major, flag);
                        }

                        if (cnt_response1 == 0)
                        {
                            semantic_error(program, parameter2->value, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    list_destroy(response1);
                }
            }
        }
        else
        if (node2->kind == NODE_KIND_CLASS)
        {
            if (entity1->type != NULL)
            {
                list_t *response1 = list_create();
                if (response1 == NULL)
                {
                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                    return -1;
                }

                int32_t r1 = semantic_cselect(program, node2, entity1->type, response1, flag);
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
                        if (item1->kind == NODE_KIND_PROPERTY)
                        {
                            list_t *response2 = list_create();
                            if (response2 == NULL)
                            {
                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r2 = semantic_vresolve(program, item1, response2, flag);
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
                                    entity1->value_update = item2;

                                    list_destroy(response2);
                                    list_destroy(response1);
                                    return 1;
                                }

                                if (cnt_response2 == 0)
                                {
                                    semantic_error(program, item1, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                                    return -1;
                                }
                            }

                            list_destroy(response2);
                        }
                        
                    }

                    if (cnt_response1 == 0)
                    {
                        if (entity1->value != NULL)
                        {
                            list_t *response2 = list_create();
                            if (response2 == NULL)
                            {
                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r2 = semantic_vresolve(program, entity1->value, response2, flag);
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
                                    entity1->value_update = item2;

                                    list_destroy(response2);
                                    list_destroy(response1);
                                    return 1;
                                }

                                if (cnt_response2 == 0)
                                {
                                    semantic_error(program, entity1->value, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                                    return -1;
                                }
                            }

                            list_destroy(response2);
                        }
                        else
                        {
                            semantic_error(program, entity1->type, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
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
                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                    return -1;
                }

                int32_t r1 = semantic_cselect(program, node2, entity1->key, response1, flag);
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
                        if (item1->kind == NODE_KIND_PROPERTY)
                        {
                            list_t *response2 = list_create();
                            if (response2 == NULL)
                            {
                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r2 = semantic_vresolve(program, item1, response2, flag);
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
                                    entity1->value_update = item2;

                                    list_destroy(response2);
                                    list_destroy(response1);
                                    return 1;
                                }

                                if (cnt_response2 == 0)
                                {
                                    semantic_error(program, item1, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                                    return -1;
                                }
                            }

                            list_destroy(response2);
                        }
                        
                    }

                    if (cnt_response1 == 0)
                    {
                        if (entity1->value != NULL)
                        {
                            list_t *response2 = list_create();
                            if (response2 == NULL)
                            {
                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r2 = semantic_vresolve(program, entity1->value, response2, flag);
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
                                    entity1->value_update = item2;
                                    
                                    list_destroy(response2);
                                    list_destroy(response1);
                                    return 1;
                                }

                                if (cnt_response2 == 0)
                                {
                                    semantic_error(program, entity1->value, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                                    return -1;
                                }
                            }

                            list_destroy(response2);
                        }
                        else
                        {
                            semantic_error(program, entity1->key, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                            return -1;
                        }
                    }
                }

                list_destroy(response1);
            }
        }
        else
        if (node2->kind == NODE_KIND_OBJECT)
        {
            if (entity1->type != NULL)
            {
                list_t *response1 = list_create();
                if (response1 == NULL)
                {
                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                    return -1;
                }

                int32_t r1 = semantic_oselect(program, node2, entity1->type, response1, flag);
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
                        if (item1->kind == NODE_KIND_PAIR)
                        {
                            list_t *response2 = list_create();
                            if (response2 == NULL)
                            {
                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r2 = semantic_vresolve(program, item1, response2, flag);
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
                                    entity1->value_update = item2;

                                    list_destroy(response2);
                                    list_destroy(response1);
                                    return 1;
                                }

                                if (cnt_response2 == 0)
                                {
                                    semantic_error(program, item1, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                                    return -1;
                                }
                            }

                            list_destroy(response2);
                        }
                    }

                    if (cnt_response1 == 0)
                    {
                        if (entity1->value != NULL)
                        {
                            list_t *response2 = list_create();
                            if (response2 == NULL)
                            {
                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r2 = semantic_vresolve(program, entity1->value, response2, flag);
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
                                    entity1->value_update = item2;

                                    list_destroy(response2);
                                    list_destroy(response1);
                                    return 1;
                                }

                                if (cnt_response2 == 0)
                                {
                                    semantic_error(program, entity1->value, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                                    return -1;
                                }
                            }

                            list_destroy(response2);
                        }
                        else
                        {
                            semantic_error(program, entity1->type, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
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
                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                    return -1;
                }

                int32_t r1 = semantic_oselect(program, node2, entity1->key, response1, flag);
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
                        if (item1->kind == NODE_KIND_PAIR)
                        {
                            list_t *response2 = list_create();
                            if (response2 == NULL)
                            {
                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r2 = semantic_vresolve(program, item1, response2, flag);
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
                                    entity1->value_update = item2;

                                    list_destroy(response2);
                                    list_destroy(response1);
                                    return 1;
                                }

                                if (cnt_response2 == 0)
                                {
                                    semantic_error(program, item1, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                                    return -1;
                                }
                            }

                            list_destroy(response2);
                        }
                    }

                    if (cnt_response1 == 0)
                    {
                        if (entity1->value != NULL)
                        {
                            list_t *response2 = list_create();
                            if (response2 == NULL)
                            {
                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }

                            int32_t r2 = semantic_vresolve(program, entity1->value, response2, flag);
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
                                    entity1->value_update = item2;

                                    list_destroy(response2);
                                    list_destroy(response1);
                                    return 1;
                                }

                                if (cnt_response2 == 0)
                                {
                                    semantic_error(program, entity1->value, "No result\n\tInternal:%s-%u", __FILE__, __LINE__);
                                    return -1;
                                }
                            }

                            list_destroy(response2);
                        }
                        else
                        {
                            semantic_error(program, entity1->key, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                            return -1;
                        }
                    }
                }
                list_destroy(response1);
            }
        }
        else
        {
            semantic_error(program, major, "It does not have a property, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
            return -1;
        }
    }
    else
    {
        semantic_error(program, major, "The type can not be set, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
            node2->position.path, node2->position.line, node2->position.column, __FILE__, __LINE__);
        return -1;
    }

    return 1;
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

        int32_t r1 = semantic_expression(program, binary1->left, response1, flag);
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
                    r2 = semantic_binary(program, node, "+", semantic_expression, response2, flag);
                }
                else
                if (node->kind == NODE_KIND_SUB_ASSIGN)
                {
                    r2 = semantic_binary(program, node, "-", semantic_expression, response2, flag);
                }
                else
                if (node->kind == NODE_KIND_MUL_ASSIGN)
                {
                    r2 = semantic_binary(program, node, "*", semantic_expression, response2, flag);
                }
                else
                if (node->kind == NODE_KIND_DIV_ASSIGN)
                {
                    r2 = semantic_binary(program, node, "/", semantic_expression, response2, flag);
                }
                else
                if (node->kind == NODE_KIND_EPI_ASSIGN)
                {
                    r2 = semantic_binary(program, node, "\\", semantic_expression, response2, flag);
                }
                else
                if (node->kind == NODE_KIND_MOD_ASSIGN)
                {
                    r2 = semantic_binary(program, node, "%", semantic_expression, response2, flag);
                }
                else
                if (node->kind == NODE_KIND_AND_ASSIGN)
                {
                    r2 = semantic_binary(program, node, "&", semantic_expression, response2, flag);
                }
                else
                if (node->kind == NODE_KIND_OR_ASSIGN)
                {
                    r2 = semantic_binary(program, node, "|", semantic_expression, response2, flag);
                }
                else
                if (node->kind == NODE_KIND_SHL_ASSIGN)
                {
                    r2 = semantic_binary(program, node, "<<", semantic_expression, response2, flag);
                }
                else
                if (node->kind == NODE_KIND_SHR_ASSIGN)
                {
                    r2 = semantic_binary(program, node, ">>", semantic_expression, response2, flag);
                }
                else
                {
                    r2 = semantic_expression(program, binary1->right, response2, flag);
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

                        int32_t r3 = semantic_assignment(program, item1, item2, node, flag);
                        if (r3 == -1)
                        {
                            return -1;
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

        int32_t r1 = semantic_expression(program, node, response1, flag);
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
