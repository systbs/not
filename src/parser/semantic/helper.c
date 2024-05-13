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

        int32_t r1 = semantic_expression(program, generic1->type, response1, flag);
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
                        node_t *key1 = generic1->key;
                        node_basic_t *key_string1 = key1->value;

                        node_class_t *class1 = (node_class_t *)item1->value;

                        node_t *key2 = class1->key;
                        node_basic_t *key_string2 = key2->value;

                        semantic_error(program, key1, "Typing:generic '%s' has an instance type of class '%s'\n\tInternal:%s-%u",
                            key_string1->value, key_string2->value, __FILE__, __LINE__);
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
                    node_t *key1 = generic1->key;
                    node_basic_t *key_string1 = key1->value;

                    semantic_error(program, generic1->key, "Typing:generic '%s' has no valid type\n\tInternal:%s-%u",
                        key_string1->value, __FILE__, __LINE__);
                    return -1;
                }
            }

            if (cnt_response1 == 0)
            {
                node_t *key1 = generic1->key;
                node_basic_t *key_string1 = key1->value;

                semantic_error(program, generic1->key, "Reference: type of generic '%s' not found\n\tInternal:%s-%u", 
                    key_string1->value, __FILE__, __LINE__);
                return -1;
            }
        }

        list_destroy(response1);
    }
    else
    {
        ilist_t *il1 = list_rpush(response, node);
        if (il1 == NULL)
        {
            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }
        return 1;
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

        int32_t r1 = semantic_expression(program, heritage1->type, response1, flag);
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
                        node_t *key1 = heritage1->key;
                        node_basic_t *key_string1 = key1->value;

                        node_class_t *class1 = (node_class_t *)item1->value;

                        node_t *key2 = class1->key;
                        node_basic_t *key_string2 = key2->value;

                        semantic_error(program, key1, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                            key_string1->value, key_string2->value, __FILE__, __LINE__);
                        return -1;
                    }

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
                else
                if (item1->kind == NODE_KIND_GENERIC)
                {
                    if ((item1->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                    {
                        node_t *key1 = heritage1->key;
                        node_basic_t *key_string1 = key1->value;

                        node_generic_t *generic1 = (node_generic_t *)item1->value;

                        node_t *key2 = generic1->key;
                        node_basic_t *key_string2 = key2->value;

                        semantic_error(program, key1, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                            key_string1->value, key_string2->value, __FILE__, __LINE__);
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
                                        node_t *key1 = generic1->key;
                                        node_basic_t *key_string1 = key1->value;

                                        node_class_t *class1 = (node_class_t *)item2->value;

                                        node_t *key2 = class1->key;
                                        node_basic_t *key_string2 = key2->value;

                                        semantic_error(program, key1, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
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
                                node_t *key1 = generic1->key;
                                node_basic_t *key_string1 = key1->value;

                                semantic_error(program, generic1->key, "Reference:type of '%s' not found\n\tInternal:%s-%u", 
                                    key_string1->value, __FILE__, __LINE__);
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
                else
                {
                    node_t *key1 = heritage1->key;
                    node_basic_t *key_string1 = key1->value;

                    semantic_error(program, heritage1->key, "Typing:'%s' has no valid type\n\tInternal:%s-%u",
                        key_string1->value, __FILE__, __LINE__);
                    return -1;
                }
            }

            if (cnt_response1 == 0)
            {
                node_t *key1 = heritage1->key;
                node_basic_t *key_string1 = key1->value;

                semantic_error(program, heritage1->key, "Reference: type of '%s' not found\n\tInternal:%s-%u", 
                    key_string1->value, __FILE__, __LINE__);
                return -1;
            }
        }

        list_destroy(response1);
    }

    return 0;
}

