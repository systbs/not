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
semantic_cselect(program_t *program, node_t *node, list_t *response, node_t *major, uint64_t flag)
{
    node_class_t *class1 = (node_class_t *)node->value;

    node_t *node1 = class1->block;
    node_block_t *block1 = (node_block_t *)node1->value;

    ilist_t *a1;
    for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        if (item1->kind == NODE_KIND_CLASS)
        {
            node_class_t *class2 = (node_class_t *)item1->value;
            if (semantic_idcmp(major, class2->key) == 1)
            {
                if ((node->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                {
                    if ((class2->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                    {
                        semantic_error(program, major, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                            class2->key->position.path, class2->key->position.line, class2->key->position.column, __FILE__, __LINE__);
                        return -1;
                    }
                }

                if ((class2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                {
                    semantic_error(program, major, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                        class2->key->position.path, class2->key->position.line, class2->key->position.column, __FILE__, __LINE__);
                    return -1;
                }

                ilist_t *r2 = list_rpush(response, item1);
                if (r2 == NULL)
                {
                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                    return -1;
                }
                continue;
            }
            continue;
        }
        else
        if (item1->kind == NODE_KIND_FUN)
        {
            node_fun_t *fun2 = (node_fun_t *)item1->value;
            if (semantic_idcmp(major, fun2->key) == 1)
            {
                if ((node->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                {
                    if ((fun2->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                    {
                        semantic_error(program, major, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                            fun2->key->position.path, fun2->key->position.line, fun2->key->position.column, __FILE__, __LINE__);
                        return -1;
                    }
                }

                if ((fun2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                {
                    semantic_error(program, major, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                        fun2->key->position.path, fun2->key->position.line, fun2->key->position.column, __FILE__, __LINE__);
                    return -1;
                }

                ilist_t *r2 = list_rpush(response, item1);
                if (r2 == NULL)
                {
                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                    return -1;
                }
                continue;
            }
            continue;
        }
        else
        if (item1->kind == NODE_KIND_PROPERTY)
        {
            node_property_t *property2 = (node_property_t *)item1->value;
            if (semantic_idcmp(major, property2->key) == 1)
            {
                if ((node->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                {
                    if ((property2->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                    {
                        semantic_error(program, major, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                            property2->key->position.path, property2->key->position.line, property2->key->position.column, __FILE__, __LINE__);
                        return -1;
                    }
                }

                if ((property2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                {
                    semantic_error(program, major, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                        property2->key->position.path, property2->key->position.line, property2->key->position.column, __FILE__, __LINE__);
                    return -1;
                }

                ilist_t *r2 = list_rpush(response, item1);
                if (r2 == NULL)
                {
                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                    return -1;
                }
                continue;
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
            node_t *item1 = (node_t *)a2->value;
            
            if (item1->kind == NODE_KIND_HERITAGE)
            {
                node_heritage_t *heritage1 = (node_heritage_t *)item1->value;

                if (heritage1->value_update != NULL)
                {
                    node_t *value_update1 = heritage1->value_update;
                    if (value_update1->kind == NODE_KIND_CLASS)
                    {
                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }

                        int32_t r1 = semantic_cselect(program, value_update1, response1, major, flag);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        {
                            ilist_t *a3;
                            for (a3 = response1->begin;a3 != response1->end;a3 = a3->next)
                            {
                                node_t *item2 = (node_t *)a2->value;

                                ilist_t *il1 = list_rpush(response, item2);
                                if (il1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                            }
                        }

                        list_destroy(response1);
                    }
                }
                else
                {
                    list_t *response1 = list_create();
                    if (response1 == NULL)
                    {
                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r1 = semantic_hresolve(program, item1, response1, flag);
                    if (r1 == -1)
                    {
                        return -1;
                    }
                    else
                    {
                        uint64_t cnt_response1 = 0;

                        ilist_t *a3;
                        for (a3 = response1->begin;a3 != response1->end;a3 = a3->next)
                        {
                            cnt_response1 += 1;

                            node_t *item2 = (node_t *)a3->value;
                            if (item2->kind == NODE_KIND_CLASS)
                            {
                                list_t *response2 = list_create();
                                if (response2 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }

                                int32_t r2 = semantic_cselect(program, item2, response2, major, flag);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                                else
                                {
                                    ilist_t *a4;
                                    for (a4 = response2->begin;a4 != response2->end;a4 = a4->next)
                                    {
                                        node_t *item3 = (node_t *)a4->value;

                                        ilist_t *il1 = list_rpush(response, item3);
                                        if (il1 == NULL)
                                        {
                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }
                                }

                                list_destroy(response2);
                            }
                        }

                        if (cnt_response1 == 0)
                        {
                            node_t *key1 = heritage1->key;
                            node_basic_t *key_string1 = key1->value;

                            semantic_error(program, heritage1->key, "Reference: type of heritage '%s' not found\n\tInternal:%s-%u", 
                                key_string1->value, __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    list_destroy(response1);
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

int32_t
semantic_attribute(program_t *program, node_t *base, node_t *node, list_t *response, uint64_t flag)
{
    node_binary_t *basic = (node_binary_t *)node->value;
    
    list_t *response1 = list_create();
    if (response1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }

    int32_t r1 = semantic_resolve(program, base, basic->left, response1, flag);
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
                list_t *response2 = list_create();
                if (response2 == NULL)
                {
                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                    return -1;
                }

                int32_t r2 = semantic_cselect(program, item1, response2, basic->right, flag);
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

                        ilist_t *il1 = list_rpush(response, item2);
                        if (il1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    if (cnt_response2 == 0)
                    {
                        node_class_t *class1 = (node_class_t *)item1->value;

                        node_t *key1 = class1->key;
                        node_basic_t *key_string1 = key1->value;

                        node_t *key2 = basic->right;
                        node_basic_t *key_string2 = key2->value;
                        
                        semantic_error(program, basic->right, "Attribute:'%s' has no attribute '%s'\n\tInternal:%s-%u", 
                            key_string1->value, key_string2->value, __FILE__, __LINE__);
                        return -1;
                    }
                }

                list_destroy(response2);
            }
            else
            if (item1->kind == NODE_KIND_GENERIC)
            {
                node_generic_t *generic1 = (node_generic_t *)item1->value;
                if (generic1->type != NULL)
                {
                    list_t *response3 = list_create();
                    if (response3 == NULL)
                    {
                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r3 = semantic_gresolve(program, item1, response3, flag);
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
                                list_t *response4 = list_create();
                                if (response4 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }

                                int32_t r3 = semantic_cselect(program, item3, response4, basic->right, flag);
                                if (r3 == -1)
                                {
                                    return -1;
                                }
                                else
                                {
                                    uint64_t cnt_response4 = 0;

                                    ilist_t *a4;
                                    for (a4 = response4->begin;a4 != response4->end;a4 = a4->next)
                                    {
                                        cnt_response4 += 1;

                                        node_t *item4 = (node_t *)a4->value;

                                        ilist_t *il1 = list_rpush(response, item4);
                                        if (il1 == NULL)
                                        {
                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }

                                    if (cnt_response4 == 0)
                                    {
                                        node_class_t *class1 = (node_class_t *)item3->value;

                                        node_t *key1 = class1->key;
                                        node_basic_t *key_string1 = key1->value;

                                        node_t *key2 = basic->right;
                                        node_basic_t *key_string2 = key2->value;
                                        
                                        semantic_error(program, basic->right, "Attribute:'%s' has no attribute '%s'\n\tInternal:%s-%u", 
                                            key_string1->value, key_string2->value, __FILE__, __LINE__);
                                        return -1;
                                    }
                                }
                                
                                list_destroy(response4);
                            }
                            else
                            if (item3->kind == NODE_KIND_GENERIC)
                            {
                                node_generic_t *generic1 = (node_generic_t *)item3->value;

                                node_t *key1 = generic1->key;
                                node_basic_t *key_string1 = key1->value;

                                node_t *key2 = basic->right;
                                node_basic_t *key_string2 = key2->value;
                                
                                semantic_error(program, basic->right, "Attribute:'%s' has no attribute '%s'\n\tInternal:%s-%u", 
                                    key_string1->value, key_string2->value, __FILE__, __LINE__);
                                return -1;
                            }
                            else
                            {
                                node_t *key1 = generic1->key;
                                node_basic_t *key_string1 = key1->value;

                                semantic_error(program, key1, "Typing:generic '%s' has no valid type\n\tInternal:%s-%u",
                                    key_string1->value, __FILE__, __LINE__);
                                return -1;
                            }
                        }

                        if (cnt_response3 == 0)
                        {
                            node_t *key1 = generic1->key;
                            node_basic_t *key_string1 = key1->value;

                            semantic_error(program, generic1->key, "Reference: type of generic '%s' not found\n\tInternal:%s-%u", 
                                key_string1->value, __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    list_destroy(response3);
                }
                else
                {
                    node_generic_t *generic1 = (node_generic_t *)item1->value;

                    node_t *key1 = generic1->key;
                    node_basic_t *key_string1 = key1->value;

                    node_t *key2 = basic->right;
                    node_basic_t *key_string2 = key2->value;
                    
                    semantic_error(program, basic->right, "Attribute:'%s' has no attribute '%s'\n\tInternal:%s-%u", 
                        key_string1->value, key_string2->value, __FILE__, __LINE__);
                    return -1;
                }
            }
            else
            if (item1->kind == NODE_KIND_VAR)
            {
                semantic_error(program, basic->right, "Typing:content cannot be accessed\n\tInternal:%s-%u", 
                    __FILE__, __LINE__);
                return -1;
            }
            else
            if (item1->kind == NODE_KIND_ENTITY)
            {
                semantic_error(program, basic->right, "Typing:content cannot be accessed\n\tInternal:%s-%u", 
                    __FILE__, __LINE__);
                return -1;
            }
            else
            if (item1->kind == NODE_KIND_PROPERTY)
            {
                semantic_error(program, basic->right, "Typing:content cannot be accessed\n\tInternal:%s-%u", 
                    __FILE__, __LINE__);
                return -1;
            }
            else
            if (item1->kind == NODE_KIND_PARAMETER)
            {
                semantic_error(program, basic->right, "Typing:content cannot be accessed\n\tInternal:%s-%u", 
                    __FILE__, __LINE__);
                return -1;
            }
            else
            if (item1->kind == NODE_KIND_PAIR)
            {
                semantic_error(program, basic->right, "Typing:content cannot be accessed\n\tInternal:%s-%u", 
                    __FILE__, __LINE__);
                return -1;
            }
            else
            if (item1->kind == NODE_KIND_HERITAGE)
            {
                semantic_error(program, basic->right, "Typing:content cannot be accessed\n\tInternal:%s-%u", 
                    __FILE__, __LINE__);
                return -1;
            }
            else
            if (item1->kind == NODE_KIND_FUN)
            {
                node_fun_t *fun1 = (node_fun_t *)item1->value;

                node_t *key1 = fun1->key;
                node_basic_t *key_string1 = key1->value;

                node_t *key2 = basic->right;
                node_basic_t *key_string2 = key2->value;

                semantic_error(program, node, "Attribute:'%s' has no attribute '%s'\n\tInternal:%s-%u", 
                    key_string1->value, key_string2->value, __FILE__, __LINE__);
                return -1;
            }
            else
            if (item1->kind == NODE_KIND_LAMBDA)
            {
                node_t *key2 = basic->right;
                node_basic_t *key_string2 = key2->value;

                semantic_error(program, node, "Attribute:'%s' has no attribute '%s'\n\tInternal:%s-%u", 
                    "lambda", key_string2->value, __FILE__, __LINE__);
                return -1;
            }
            else
			{
                node_t *key2 = basic->right;
                node_basic_t *key_string2 = key2->value;

                semantic_error(program, node, "Attribute:it does not have the attribute '%s'\n\tInternal:%s-%u", 
                    key_string2->value, __FILE__, __LINE__);
                return -1;
			}
        }

        if (cnt_response1 == 0)
        {
            semantic_error(program, node, "Reference: base of '%s' has no result\n\tInternal:%s-%u", 
                "attribute", __FILE__, __LINE__);
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
