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
semantic_cresolve(program_t *program, node_t *node, list_t *response, node_t *major, uint64_t flag)
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
                        int32_t r1 = semantic_cresolve(program, value_update1, response, major, flag);
                        if (r1 == -1)
                        {
                            return -1;
                        }
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
                                int32_t r2 = semantic_cresolve(program, item2, response, major, flag);
                                if (r2 == -1)
                                {
                                    return -1;
                                }
                            }
                        }

                        if (cnt_response1 == 0)
                        {
                            semantic_error(program, heritage1->key, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    list_destroy(response1);
                }
            }
        }
    }

    return 1;
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
                int32_t r2 = semantic_cresolve(program, item1, response, basic->right, flag);
                if (r2 == -1)
                {
                    return -1;
                }
            }
            else
            if (item1->kind == NODE_KIND_VAR)
            {
                node_var_t *var1 = (node_var_t *)item1->value;
                if (var1->value_update != NULL)
                {
                    node_t *value_update1 = var1->value_update;
                    if (value_update1->kind == NODE_KIND_CLASS)
                    {
                        int32_t r2 = semantic_cresolve(program, value_update1, response, basic->right, flag);
                        if (r2 == -1)
                        {
                            return -1;
                        }
                    }
                    else
                    {
                        semantic_error(program, basic->left, "Inaccessible\n\tInternal:%s-%u", __FILE__, __LINE__);
                        return -1;
                    }
                }
                else
                {
                    semantic_error(program, basic->left, "Not initialized\n\tInternal:%s-%u", __FILE__, __LINE__);
                    return -1;
                }
            }
            else
            if (item1->kind == NODE_KIND_ENTITY)
            {
                node_entity_t *entity1 = (node_entity_t *)item1->value;
                if (entity1->value_update != NULL)
                {
                    node_t *value_update1 = entity1->value_update;
                    if (value_update1->kind == NODE_KIND_CLASS)
                    {
                        int32_t r2 = semantic_cresolve(program, value_update1, response, basic->right, flag);
                        if (r2 == -1)
                        {
                            return -1;
                        }
                    }
                    else
                    {
                        semantic_error(program, basic->left, "Inaccessible\n\tInternal:%s-%u", __FILE__, __LINE__);
                        return -1;
                    }
                }
                else
                {
                    semantic_error(program, basic->left, "Not initialized\n\tInternal:%s-%u", __FILE__, __LINE__);
                    return -1;
                }
            }
            else
            if (item1->kind == NODE_KIND_PROPERTY)
            {
                node_property_t *property1 = (node_property_t *)item1->value;
                if (property1->value_update != NULL)
                {
                    node_t *value_update1 = property1->value_update;
                    if (value_update1->kind == NODE_KIND_CLASS)
                    {
                        int32_t r2 = semantic_cresolve(program, value_update1, response, basic->right, flag);
                        if (r2 == -1)
                        {
                            return -1;
                        }
                    }
                    else
                    {
                        semantic_error(program, basic->left, "Inaccessible\n\tInternal:%s-%u", __FILE__, __LINE__);
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
                            semantic_error(program, basic->left, "Inaccessible, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                node1->position.path, node1->position.line, node1->position.column, __FILE__, __LINE__);
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
                                        if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                        {
                                            semantic_error(program, node1, "Instance object, in confronting with (%s-%lld:%lld)",
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

                                        property1->value_update = clone1;

                                        int32_t r3 = semantic_cresolve(program, clone1, response, basic->right, flag);
                                        if (r3 == -1)
                                        {
                                            return -1;
                                        }
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
                                                    if ((item3->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                    {
                                                        semantic_error(program, generic1->type, "Instance object, in confronting with (%s-%lld:%lld)",
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

                                                    property1->value_update = clone1;

                                                    int32_t r4 = semantic_cresolve(program, clone1, response, basic->right, flag);
                                                    if (r4 == -1)
                                                    {
                                                        return -1;
                                                    }
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
                                    if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, node1, "Instance object, in confronting with (%s-%lld:%lld)",
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

                                    property1->value_update = clone1;

                                    node_class_t *class1 = (node_class_t *)clone1->value;

                                    node_t *node2 = class1->block;
                                    node_block_t *block1 = (node_block_t *)node2->value;

                                    ilist_t *a3;
                                    for (a3 = block1->list->begin;a3 != block1->list->end;a3 = a3->next)
                                    {
                                        node_t *item3 = (node_t *)a3->value;
                                        if (item3->kind == NODE_KIND_CLASS)
                                        {
                                            node_class_t *class2 = (node_class_t *)item3->value;
                                            if (semantic_idcmp(basic->right, class2->key) == 1)
                                            {
                                                if ((clone1->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                {
                                                    if ((class2->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                    {
                                                        semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                            class2->key->position.path, class2->key->position.line, class2->key->position.column, __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                }

                                                if ((class2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                {
                                                    semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                        class2->key->position.path, class2->key->position.line, class2->key->position.column, __FILE__, __LINE__);
                                                    return -1;
                                                }

                                                ilist_t *r3 = list_rpush(response, item3);
                                                if (r3 == NULL)
                                                {
                                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                continue;
                                            }
                                        }
                                        else
                                        if (item3->kind == NODE_KIND_FUN)
                                        {
                                            node_fun_t *fun2 = (node_fun_t *)item3->value;
                                            if (semantic_idcmp(basic->right, fun2->key) == 1)
                                            {
                                                if ((clone1->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                {
                                                    if ((fun2->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                    {
                                                        semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                            fun2->key->position.path, fun2->key->position.line, fun2->key->position.column, __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                }

                                                if ((fun2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                {
                                                    semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                        fun2->key->position.path, fun2->key->position.line, fun2->key->position.column, __FILE__, __LINE__);
                                                    return -1;
                                                }

                                                ilist_t *r3 = list_rpush(response, item3);
                                                if (r3 == NULL)
                                                {
                                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                continue;
                                            }
                                        }
                                        else
                                        if (item2->kind == NODE_KIND_PROPERTY)
                                        {
                                            node_property_t *property2 = (node_property_t *)item2->value;
                                            if (semantic_idcmp(basic->right, property2->key) == 1)
                                            {
                                                if ((clone1->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                {
                                                    if ((property2->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                    {
                                                        semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                            property2->key->position.path, property2->key->position.line, property2->key->position.column, __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                }

                                                if ((property2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                {
                                                    semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                        property2->key->position.path, property2->key->position.line, property2->key->position.column, __FILE__, __LINE__);
                                                    return -1;
                                                }

                                                ilist_t *r3 = list_rpush(response, item3);
                                                if (r3 == NULL)
                                                {
                                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                continue;
                                            }
                                        }
                                    }

                                    if (class1->heritages != NULL)
                                    {
                                        node_t *node3 = class1->heritages;

                                        list_t *repository1 = list_create();
                                        if (repository1 == NULL)
                                        {
                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }

                                        while (node3 != NULL)
                                        {
                                            node_block_t *block2 = (node_block_t *)node3->value;

                                            ilist_t *a4;
                                            for (a4 = block2->list->begin;a4 != block2->list->end;a4 = a4->next)
                                            {
                                                node_t *item3 = (node_t *)a4->value;
                                                
                                                if (item3->kind == NODE_KIND_HERITAGE)
                                                {
                                                    node_heritage_t *heritage1 = (node_heritage_t *)item3->value;
                                                
                                                    if (heritage1->value_update == NULL)
                                                    {
                                                        list_t *response3 = list_create();
                                                        if (response3 == NULL)
                                                        {
                                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                    
                                                        int32_t r3 = semantic_resolve(program, heritage1->type->parent, heritage1->type, response3, SEMANTIC_FLAG_NONE);
                                                        if (r3 == -1)
                                                        {
                                                            return -1;
                                                        }
                                                        else
                                                        {
                                                            uint64_t cnt_response3 = 0;

                                                            ilist_t *a5;
                                                            for (a5 = response3->begin;a5 != response3->end;a5 = a5->next)
                                                            {
                                                                cnt_response3 += 1;

                                                                node_t *item4 = (node_t *)a5->value;

                                                                if (item4->kind == NODE_KIND_CLASS)
                                                                {
                                                                    if ((item4->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                                    {
                                                                        semantic_error(program, heritage1->type, "Instance object, in confronting with (%s-%lld:%lld)",
                                                                            item4->position.path, item4->position.line, item4->position.column);
                                                                        return -1;
                                                                    }

                                                                    node_t *clone2 = node_clone(item4->parent, item4);
                                                                    if (clone2 == NULL)
                                                                    {
                                                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                        return -1;
                                                                    }
                                                                    clone2->flag |= NODE_FLAG_INSTANCE;

                                                                    heritage1->value_update = clone2;

                                                                    node_class_t *class2 = (node_class_t *)clone2->value;

                                                                    node_t *node4 = class2->block;
                                                                    node_block_t *block3 = (node_block_t *)node4->value;

                                                                    ilist_t *a6;
                                                                    for (a6 = block3->list->begin;a6 != block3->list->end;a6 = a6->next)
                                                                    {
                                                                        node_t *item5 = (node_t *)a6->value;
                                                                        if (item5->kind == NODE_KIND_CLASS)
                                                                        {
                                                                            node_class_t *class3 = (node_class_t *)item5->value;
                                                                            if (semantic_idcmp(class3->key, basic->right) == 1)
                                                                            {
                                                                                if ((clone2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                                {
                                                                                    if ((class3->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                                    {
                                                                                        semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                            class3->key->position.path, class3->key->position.line, class3->key->position.column, __FILE__, __LINE__);
                                                                                        return -1;
                                                                                    }
                                                                                }

                                                                                if ((class3->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                {
                                                                                    semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                        class3->key->position.path, class3->key->position.line, class3->key->position.column, __FILE__, __LINE__);
                                                                                    return -1;
                                                                                }

                                                                                ilist_t *r4 = list_rpush(response, item4);
                                                                                if (r4 == NULL)
                                                                                {
                                                                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                    return -1;
                                                                                }
                                                                                continue;
                                                                            }
                                                                        }
                                                                        else
                                                                        if (item5->kind == NODE_KIND_FUN)
                                                                        {
                                                                            node_fun_t *fun1 = (node_fun_t *)item5->value;
                                                                            if (semantic_idcmp(fun1->key, basic->right) == 1)
                                                                            {
                                                                                if ((clone2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                                {
                                                                                    if ((fun1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                                    {
                                                                                        semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                            fun1->key->position.path, fun1->key->position.line, fun1->key->position.column, __FILE__, __LINE__);
                                                                                        return -1;
                                                                                    }
                                                                                }

                                                                                if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                {
                                                                                    semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                        fun1->key->position.path, fun1->key->position.line, fun1->key->position.column, __FILE__, __LINE__);
                                                                                    return -1;
                                                                                }

                                                                                ilist_t *r4 = list_rpush(response, item4);
                                                                                if (r4 == NULL)
                                                                                {
                                                                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                    return -1;
                                                                                }
                                                                                continue;
                                                                            }
                                                                        }
                                                                        else
                                                                        if (item5->kind == NODE_KIND_PROPERTY)
                                                                        {
                                                                            node_property_t *property2 = (node_property_t *)item5->value;
                                                                            if (semantic_idcmp(property2->key, basic->right) == 1)
                                                                            {
                                                                                if ((clone2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                                {
                                                                                    if ((property2->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                                    {
                                                                                        semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                            property2->key->position.path, property2->key->position.line, property2->key->position.column, __FILE__, __LINE__);
                                                                                        return -1;
                                                                                    }
                                                                                }

                                                                                if ((property2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                {
                                                                                    semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                        property2->key->position.path, property2->key->position.line, property2->key->position.column, __FILE__, __LINE__);
                                                                                    return -1;
                                                                                }

                                                                                ilist_t *r4 = list_rpush(response, item4);
                                                                                if (r4 == NULL)
                                                                                {
                                                                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                    return -1;
                                                                                }
                                                                                continue;
                                                                            }
                                                                        }
                                                                    }

                                                                    ilist_t *r4 = list_rpush(repository1, class2->heritages);
                                                                    if (r4 == NULL)
                                                                    {
                                                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                        return -1;
                                                                    }
                                                                }
                                                                else
                                                                if (item4->kind == NODE_KIND_GENERIC)
                                                                {
                                                                    node_t *node4 = item4;
                                                                    while (node4 != NULL)
                                                                    {
                                                                        node_generic_t *generic1 = (node_generic_t *)node4->value;
                                                                        if (generic1->type != NULL)
                                                                        {
                                                                            list_t *response4 = list_create();
                                                                            if (response4 == NULL)
                                                                            {
                                                                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                return -1;
                                                                            }
                                                                        
                                                                            int32_t r4 = semantic_resolve(program, generic1->type->parent, generic1->type, response4, SEMANTIC_FLAG_NONE);
                                                                            if (r4 == -1)
                                                                            {
                                                                                return -1;
                                                                            }
                                                                            else
                                                                            {
                                                                                uint64_t cnt_response4 = 0;

                                                                                ilist_t *a6;
                                                                                for (a6 = response4->begin;a6 != response4->end;a6 = a6->next)
                                                                                {
                                                                                    cnt_response4 += 1;

                                                                                    node_t *item5 = (node_t *)a6->value;
                                                                                    if (item5->kind == NODE_KIND_CLASS)
                                                                                    {
                                                                                        if ((item5->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                                                        {
                                                                                            semantic_error(program, generic1->type, "Instance object, in confronting with (%s-%lld:%lld)",
                                                                                                item5->position.path, item5->position.line, item5->position.column);
                                                                                            return -1;
                                                                                        }

                                                                                        node_t *clone2 = node_clone(item5->parent, item5);
                                                                                        if (clone2 == NULL)
                                                                                        {
                                                                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                            return -1;
                                                                                        }
                                                                                        clone2->flag |= NODE_FLAG_INSTANCE;

                                                                                        heritage1->value_update = clone2;

                                                                                        node_class_t *class2 = (node_class_t *)clone2->value;

                                                                                        node_t *node5 = class2->block;
                                                                                        node_block_t *block3 = (node_block_t *)node5->value;

                                                                                        ilist_t *a7;
                                                                                        for (a7 = block3->list->begin;a7 != block3->list->end;a7 = a7->next)
                                                                                        {
                                                                                            node_t *item6 = (node_t *)a7->value;
                                                                                            if (item6->kind == NODE_KIND_CLASS)
                                                                                            {
                                                                                                node_class_t *class3 = (node_class_t *)item6->value;
                                                                                                if (semantic_idcmp(class3->key, basic->right) == 1)
                                                                                                {
                                                                                                    if ((clone2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                                                    {
                                                                                                        if ((class3->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                                                        {
                                                                                                            semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                                class3->key->position.path, class3->key->position.line, class3->key->position.column, __FILE__, __LINE__);
                                                                                                            return -1;
                                                                                                        }
                                                                                                    }

                                                                                                    if ((class3->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                                    {
                                                                                                        semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                            class3->key->position.path, class3->key->position.line, class3->key->position.column, __FILE__, __LINE__);
                                                                                                        return -1;
                                                                                                    }

                                                                                                    ilist_t *r5 = list_rpush(response, item6);
                                                                                                    if (r5 == NULL)
                                                                                                    {
                                                                                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                                        return -1;
                                                                                                    }
                                                                                                    continue;
                                                                                                }
                                                                                            }
                                                                                            else
                                                                                            if (item6->kind == NODE_KIND_FUN)
                                                                                            {
                                                                                                node_fun_t *fun1 = (node_fun_t *)item6->value;
                                                                                                if (semantic_idcmp(fun1->key, basic->right) == 1)
                                                                                                {
                                                                                                    if ((clone2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                                                    {
                                                                                                        if ((fun1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                                                        {
                                                                                                            semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                                fun1->key->position.path, fun1->key->position.line, fun1->key->position.column, __FILE__, __LINE__);
                                                                                                            return -1;
                                                                                                        }
                                                                                                    }

                                                                                                    if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                                    {
                                                                                                        semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                            fun1->key->position.path, fun1->key->position.line, fun1->key->position.column, __FILE__, __LINE__);
                                                                                                        return -1;
                                                                                                    }

                                                                                                    ilist_t *r5 = list_rpush(response, item6);
                                                                                                    if (r5 == NULL)
                                                                                                    {
                                                                                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                                        return -1;
                                                                                                    }
                                                                                                    continue;
                                                                                                }
                                                                                            }
                                                                                            else
                                                                                            if (item6->kind == NODE_KIND_PROPERTY)
                                                                                            {
                                                                                                node_property_t *property2 = (node_property_t *)item6->value;
                                                                                                if (semantic_idcmp(property2->key, basic->right) == 1)
                                                                                                {
                                                                                                    if ((clone2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                                                    {
                                                                                                        if ((property2->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                                                        {
                                                                                                            semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                                property2->key->position.path, property2->key->position.line, property2->key->position.column, __FILE__, __LINE__);
                                                                                                            return -1;
                                                                                                        }
                                                                                                    }

                                                                                                    if ((property2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                                    {
                                                                                                        semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                            property2->key->position.path, property2->key->position.line, property2->key->position.column, __FILE__, __LINE__);
                                                                                                        return -1;
                                                                                                    }

                                                                                                    ilist_t *r5 = list_rpush(response, item6);
                                                                                                    if (r5 == NULL)
                                                                                                    {
                                                                                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                                        return -1;
                                                                                                    }
                                                                                                    continue;
                                                                                                }
                                                                                            }
                                                                                        }

                                                                                        ilist_t *r5 = list_rpush(repository1, class2->heritages);
                                                                                        if (r5 == NULL)
                                                                                        {
                                                                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                            return -1;
                                                                                        }

                                                                                    }
                                                                                    else
                                                                                    if (item5->kind == NODE_KIND_GENERIC)
                                                                                    {
                                                                                        node4 = item5;
                                                                                        break;
                                                                                    }
                                                                                    else
                                                                                    {
                                                                                        semantic_error(program, generic1->type, "Wrong type, in confronting with (%s-%lld:%lld)",
                                                                                            item5->position.path, item5->position.line, item5->position.column);
                                                                                        return -1;
                                                                                    }
                                                                                }

                                                                                if (cnt_response4 == 0)
                                                                                {
                                                                                    semantic_error(program, generic1->type, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                                                                    return -1;
                                                                                }
                                                                            }
                                                                            list_destroy(response4);
                                                                        }
                                                                    }
                                                                }
                                                                else
                                                                {
                                                                    semantic_error(program, heritage1->type, "Wrong type, in confronting with (%s-%lld:%lld)",
                                                                        item4->position.path, item4->position.line, item4->position.column);
                                                                    return -1;
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
                                                    else
                                                    {
                                                        node_t *node4 = heritage1->value_update;

                                                        if (node4->kind == NODE_KIND_CLASS)
                                                        {
                                                            node_class_t *class2 = (node_class_t *)node4->value;

                                                            node_t *node5 = class2->block;
                                                            node_block_t *block3 = (node_block_t *)node5->value;

                                                            ilist_t *a5;
                                                            for (a5 = block3->list->begin;a5 != block3->list->end;a5 = a5->next)
                                                            {
                                                                node_t *item4 = (node_t *)a5->value;
                                                                if (item4->kind == NODE_KIND_CLASS)
                                                                {
                                                                    node_class_t *class3 = (node_class_t *)item4->value;
                                                                    if (semantic_idcmp(class3->key, basic->right) == 1)
                                                                    {
                                                                        if ((node4->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                        {
                                                                            if ((class3->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                            {
                                                                                semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                    class3->key->position.path, class3->key->position.line, class3->key->position.column, __FILE__, __LINE__);
                                                                                return -1;
                                                                            }
                                                                        }

                                                                        if ((class3->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                        {
                                                                            semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                class3->key->position.path, class3->key->position.line, class3->key->position.column, __FILE__, __LINE__);
                                                                            return -1;
                                                                        }

                                                                        ilist_t *r3 = list_rpush(response, item4);
                                                                        if (r3 == NULL)
                                                                        {
                                                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                            return -1;
                                                                        }
                                                                        continue;
                                                                    }
                                                                }
                                                                else
                                                                if (item4->kind == NODE_KIND_FUN)
                                                                {
                                                                    node_fun_t *fun1 = (node_fun_t *)item4->value;
                                                                    if (semantic_idcmp(fun1->key, basic->right) == 1)
                                                                    {
                                                                        if ((node4->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                        {
                                                                            if ((fun1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                            {
                                                                                semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                    fun1->key->position.path, fun1->key->position.line, fun1->key->position.column, __FILE__, __LINE__);
                                                                                return -1;
                                                                            }
                                                                        }

                                                                        if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                        {
                                                                            semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                fun1->key->position.path, fun1->key->position.line, fun1->key->position.column, __FILE__, __LINE__);
                                                                            return -1;
                                                                        }

                                                                        ilist_t *r3 = list_rpush(response, item4);
                                                                        if (r3 == NULL)
                                                                        {
                                                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                            return -1;
                                                                        }
                                                                        continue;
                                                                    }
                                                                }
                                                                else
                                                                if (item4->kind == NODE_KIND_PROPERTY)
                                                                {
                                                                    node_property_t *property2 = (node_property_t *)item4->value;
                                                                    if (semantic_idcmp(property2->key, basic->right) == 1)
                                                                    {
                                                                        if ((node4->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                        {
                                                                            if ((property2->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                            {
                                                                                semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                    property2->key->position.path, property2->key->position.line, property2->key->position.column, __FILE__, __LINE__);
                                                                                return -1;
                                                                            }
                                                                        }

                                                                        if ((property2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                        {
                                                                            semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                property2->key->position.path, property2->key->position.line, property2->key->position.column, __FILE__, __LINE__);
                                                                            return -1;
                                                                        }

                                                                        ilist_t *r3 = list_rpush(response, item4);
                                                                        if (r3 == NULL)
                                                                        {
                                                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                            return -1;
                                                                        }
                                                                        continue;
                                                                    }
                                                                }
                                                            }

                                                            ilist_t *r3 = list_rpush(repository1, class2->heritages);
                                                            if (r3 == NULL)
                                                            {
                                                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                return -1;
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        
                                            ilist_t *r3 = list_rpop(repository1);
                                            if (r3 != NULL)
                                            {
                                                node3 = (node_t *)r3->value;
                                                continue;
                                            }
                                            else
                                            {
                                                node3 = NULL;
                                                break;
                                            }
                                        }

                                        list_destroy(repository1);
                                    }
                                }
                                else
                                if (item2->kind == NODE_KIND_GENERIC)
                                {
                                    node_t *node2 = item2;
                                    while (node2 != NULL)
                                    {
                                        node_generic_t *generic1 = (node_generic_t *)node2->value;

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
                                                        if ((item3->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                        {
                                                            semantic_error(program, generic1->type, "Instance object, in confronting with (%s-%lld:%lld)",
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

                                                        property1->value_update = clone1;

                                                        node_class_t *class1 = (node_class_t *)clone1->value;

                                                        node_t *node3 = class1->block;
                                                        node_block_t *block1 = (node_block_t *)node3->value;

                                                        ilist_t *a4;
                                                        for (a4 = block1->list->begin;a4 != block1->list->end;a4 = a4->next)
                                                        {
                                                            node_t *item4 = (node_t *)a4->value;
                                                            if (item4->kind == NODE_KIND_CLASS)
                                                            {
                                                                node_class_t *class2 = (node_class_t *)item4->value;
                                                                if (semantic_idcmp(basic->right, class2->key) == 1)
                                                                {
                                                                    if ((clone1->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                    {
                                                                        if ((class2->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                        {
                                                                            semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                class2->key->position.path, class2->key->position.line, class2->key->position.column, __FILE__, __LINE__);
                                                                            return -1;
                                                                        }
                                                                    }

                                                                    if ((class2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                    {
                                                                        semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                            class2->key->position.path, class2->key->position.line, class2->key->position.column, __FILE__, __LINE__);
                                                                        return -1;
                                                                    }

                                                                    ilist_t *r4 = list_rpush(response, item4);
                                                                    if (r4 == NULL)
                                                                    {
                                                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                        return -1;
                                                                    }
                                                                    continue;
                                                                }
                                                            }
                                                            else
                                                            if (item4->kind == NODE_KIND_FUN)
                                                            {
                                                                node_fun_t *fun2 = (node_fun_t *)item4->value;
                                                                if (semantic_idcmp(basic->right, fun2->key) == 1)
                                                                {
                                                                    if ((clone1->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                    {
                                                                        if ((fun2->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                        {
                                                                            semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                fun2->key->position.path, fun2->key->position.line, fun2->key->position.column, __FILE__, __LINE__);
                                                                            return -1;
                                                                        }
                                                                    }

                                                                    if ((fun2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                    {
                                                                        semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                            fun2->key->position.path, fun2->key->position.line, fun2->key->position.column, __FILE__, __LINE__);
                                                                        return -1;
                                                                    }

                                                                    ilist_t *r4 = list_rpush(response, item4);
                                                                    if (r4 == NULL)
                                                                    {
                                                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                        return -1;
                                                                    }
                                                                    continue;
                                                                }
                                                            }
                                                            else
                                                            if (item4->kind == NODE_KIND_PROPERTY)
                                                            {
                                                                node_property_t *property2 = (node_property_t *)item4->value;
                                                                if (semantic_idcmp(basic->right, property2->key) == 1)
                                                                {
                                                                    if ((clone1->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                    {
                                                                        if ((property2->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                        {
                                                                            semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                property2->key->position.path, property2->key->position.line, property2->key->position.column, __FILE__, __LINE__);
                                                                            return -1;
                                                                        }
                                                                    }

                                                                    if ((property2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                    {
                                                                        semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                            property2->key->position.path, property2->key->position.line, property2->key->position.column, __FILE__, __LINE__);
                                                                        return -1;
                                                                    }

                                                                    ilist_t *r4 = list_rpush(response, item4);
                                                                    if (r4 == NULL)
                                                                    {
                                                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                        return -1;
                                                                    }
                                                                    continue;
                                                                }
                                                            }
                                                        }

                                                        if (class1->heritages != NULL)
                                                        {
                                                            node_t *node4 = class1->heritages;

                                                            list_t *repository1 = list_create();
                                                            if (repository1 == NULL)
                                                            {
                                                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                return -1;
                                                            }

                                                            while (node4 != NULL)
                                                            {
                                                                node_block_t *block2 = (node_block_t *)node4->value;

                                                                ilist_t *a5;
                                                                for (a5 = block2->list->begin;a5 != block2->list->end;a5 = a5->next)
                                                                {
                                                                    node_t *item4 = (node_t *)a5->value;
                                                                    
                                                                    if (item4->kind == NODE_KIND_HERITAGE)
                                                                    {
                                                                        node_heritage_t *heritage1 = (node_heritage_t *)item4->value;
                                                                    
                                                                        if (heritage1->value_update == NULL)
                                                                        {
                                                                            list_t *response4 = list_create();
                                                                            if (response4 == NULL)
                                                                            {
                                                                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                return -1;
                                                                            }
                                                                        
                                                                            int32_t r4 = semantic_resolve(program, heritage1->type->parent, heritage1->type, response4, SEMANTIC_FLAG_NONE);
                                                                            if (r4 == -1)
                                                                            {
                                                                                return -1;
                                                                            }
                                                                            else
                                                                            {
                                                                                uint64_t cnt_response4 = 0;

                                                                                ilist_t *a6;
                                                                                for (a6 = response4->begin;a6 != response4->end;a6 = a6->next)
                                                                                {
                                                                                    cnt_response4 += 1;

                                                                                    node_t *item5 = (node_t *)a6->value;

                                                                                    if (item5->kind == NODE_KIND_CLASS)
                                                                                    {
                                                                                        if ((item5->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                                                        {
                                                                                            semantic_error(program, heritage1->type, "Instance object, in confronting with (%s-%lld:%lld)",
                                                                                                item5->position.path, item5->position.line, item5->position.column);
                                                                                            return -1;
                                                                                        }

                                                                                        node_t *clone2 = node_clone(item5->parent, item5);
                                                                                        if (clone2 == NULL)
                                                                                        {
                                                                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                            return -1;
                                                                                        }
                                                                                        clone2->flag |= NODE_FLAG_INSTANCE;

                                                                                        heritage1->value_update = clone2;

                                                                                        node_class_t *class2 = (node_class_t *)clone2->value;

                                                                                        node_t *node5 = class2->block;
                                                                                        node_block_t *block3 = (node_block_t *)node5->value;

                                                                                        ilist_t *a7;
                                                                                        for (a7 = block3->list->begin;a7 != block3->list->end;a7 = a7->next)
                                                                                        {
                                                                                            node_t *item6 = (node_t *)a7->value;
                                                                                            if (item6->kind == NODE_KIND_CLASS)
                                                                                            {
                                                                                                node_class_t *class3 = (node_class_t *)item6->value;
                                                                                                if (semantic_idcmp(class3->key, basic->right) == 1)
                                                                                                {
                                                                                                    if ((clone2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                                                    {
                                                                                                        if ((class3->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                                                        {
                                                                                                            semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                                class3->key->position.path, class3->key->position.line, class3->key->position.column, __FILE__, __LINE__);
                                                                                                            return -1;
                                                                                                        }
                                                                                                    }

                                                                                                    if ((class3->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                                    {
                                                                                                        semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                            class3->key->position.path, class3->key->position.line, class3->key->position.column, __FILE__, __LINE__);
                                                                                                        return -1;
                                                                                                    }

                                                                                                    ilist_t *r5 = list_rpush(response, item6);
                                                                                                    if (r5 == NULL)
                                                                                                    {
                                                                                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                                        return -1;
                                                                                                    }
                                                                                                    continue;
                                                                                                }
                                                                                            }
                                                                                            else
                                                                                            if (item6->kind == NODE_KIND_FUN)
                                                                                            {
                                                                                                node_fun_t *fun1 = (node_fun_t *)item6->value;
                                                                                                if (semantic_idcmp(fun1->key, basic->right) == 1)
                                                                                                {
                                                                                                    if ((clone2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                                                    {
                                                                                                        if ((fun1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                                                        {
                                                                                                            semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                                fun1->key->position.path, fun1->key->position.line, fun1->key->position.column, __FILE__, __LINE__);
                                                                                                            return -1;
                                                                                                        }
                                                                                                    }

                                                                                                    if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                                    {
                                                                                                        semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                            fun1->key->position.path, fun1->key->position.line, fun1->key->position.column, __FILE__, __LINE__);
                                                                                                        return -1;
                                                                                                    }

                                                                                                    ilist_t *r5 = list_rpush(response, item6);
                                                                                                    if (r5 == NULL)
                                                                                                    {
                                                                                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                                        return -1;
                                                                                                    }
                                                                                                    continue;
                                                                                                }
                                                                                            }
                                                                                            else
                                                                                            if (item6->kind == NODE_KIND_PROPERTY)
                                                                                            {
                                                                                                node_property_t *property2 = (node_property_t *)item6->value;
                                                                                                if (semantic_idcmp(property2->key, basic->right) == 1)
                                                                                                {
                                                                                                    if ((clone2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                                                    {
                                                                                                        if ((property2->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                                                        {
                                                                                                            semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                                property2->key->position.path, property2->key->position.line, property2->key->position.column, __FILE__, __LINE__);
                                                                                                            return -1;
                                                                                                        }
                                                                                                    }

                                                                                                    if ((property2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                                    {
                                                                                                        semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                            property2->key->position.path, property2->key->position.line, property2->key->position.column, __FILE__, __LINE__);
                                                                                                        return -1;
                                                                                                    }

                                                                                                    ilist_t *r5 = list_rpush(response, item6);
                                                                                                    if (r5 == NULL)
                                                                                                    {
                                                                                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                                        return -1;
                                                                                                    }
                                                                                                    continue;
                                                                                                }
                                                                                            }
                                                                                        }

                                                                                        ilist_t *r5 = list_rpush(repository1, class2->heritages);
                                                                                        if (r5 == NULL)
                                                                                        {
                                                                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                            return -1;
                                                                                        }
                                                                                    }
                                                                                    else
                                                                                    if (item5->kind == NODE_KIND_GENERIC)
                                                                                    {
                                                                                        node_t *node5 = item5;
                                                                                        while (node5 != NULL)
                                                                                        {
                                                                                            node_generic_t *generic2 = (node_generic_t *)node5->value;
                                                                                            if (generic2->type != NULL)
                                                                                            {
                                                                                                list_t *response5 = list_create();
                                                                                                if (response5 == NULL)
                                                                                                {
                                                                                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                                    return -1;
                                                                                                }
                                                                                            
                                                                                                int32_t r5 = semantic_resolve(program, generic2->type->parent, generic2->type, response5, SEMANTIC_FLAG_NONE);
                                                                                                if (r5 == -1)
                                                                                                {
                                                                                                    return -1;
                                                                                                }
                                                                                                else
                                                                                                {
                                                                                                    uint64_t cnt_response5 = 0;

                                                                                                    ilist_t *a7;
                                                                                                    for (a7 = response5->begin;a7 != response5->end;a7 = a7->next)
                                                                                                    {
                                                                                                        cnt_response5 += 1;

                                                                                                        node_t *item6 = (node_t *)a7->value;
                                                                                                        if (item6->kind == NODE_KIND_CLASS)
                                                                                                        {
                                                                                                            if ((item6->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                                                                            {
                                                                                                                semantic_error(program, generic1->type, "Instance object, in confronting with (%s-%lld:%lld)",
                                                                                                                    item6->position.path, item6->position.line, item6->position.column);
                                                                                                                return -1;
                                                                                                            }

                                                                                                            node_t *clone2 = node_clone(item6->parent, item6);
                                                                                                            if (clone2 == NULL)
                                                                                                            {
                                                                                                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                                                return -1;
                                                                                                            }
                                                                                                            clone2->flag |= NODE_FLAG_INSTANCE;

                                                                                                            heritage1->value_update = clone2;

                                                                                                            node_class_t *class2 = (node_class_t *)clone2->value;

                                                                                                            node_t *node6 = class2->block;
                                                                                                            node_block_t *block3 = (node_block_t *)node6->value;

                                                                                                            ilist_t *a8;
                                                                                                            for (a8 = block3->list->begin;a8 != block3->list->end;a8 = a8->next)
                                                                                                            {
                                                                                                                node_t *item7 = (node_t *)a8->value;
                                                                                                                if (item7->kind == NODE_KIND_CLASS)
                                                                                                                {
                                                                                                                    node_class_t *class3 = (node_class_t *)item7->value;
                                                                                                                    if (semantic_idcmp(class3->key, basic->right) == 1)
                                                                                                                    {
                                                                                                                        if ((clone2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                                                                        {
                                                                                                                            if ((class3->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                                                                            {
                                                                                                                                semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                                                    class3->key->position.path, class3->key->position.line, class3->key->position.column, __FILE__, __LINE__);
                                                                                                                                return -1;
                                                                                                                            }
                                                                                                                        }

                                                                                                                        if ((class3->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                                                        {
                                                                                                                            semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                                                class3->key->position.path, class3->key->position.line, class3->key->position.column, __FILE__, __LINE__);
                                                                                                                            return -1;
                                                                                                                        }

                                                                                                                        ilist_t *r6 = list_rpush(response, item7);
                                                                                                                        if (r6 == NULL)
                                                                                                                        {
                                                                                                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                                                            return -1;
                                                                                                                        }
                                                                                                                        continue;
                                                                                                                    }
                                                                                                                }
                                                                                                                else
                                                                                                                if (item7->kind == NODE_KIND_FUN)
                                                                                                                {
                                                                                                                    node_fun_t *fun1 = (node_fun_t *)item7->value;
                                                                                                                    if (semantic_idcmp(fun1->key, basic->right) == 1)
                                                                                                                    {
                                                                                                                        if ((clone2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                                                                        {
                                                                                                                            if ((fun1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                                                                            {
                                                                                                                                semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                                                    fun1->key->position.path, fun1->key->position.line, fun1->key->position.column, __FILE__, __LINE__);
                                                                                                                                return -1;
                                                                                                                            }
                                                                                                                        }

                                                                                                                        if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                                                        {
                                                                                                                            semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                                                fun1->key->position.path, fun1->key->position.line, fun1->key->position.column, __FILE__, __LINE__);
                                                                                                                            return -1;
                                                                                                                        }

                                                                                                                        ilist_t *r6 = list_rpush(response, item7);
                                                                                                                        if (r6 == NULL)
                                                                                                                        {
                                                                                                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                                                            return -1;
                                                                                                                        }
                                                                                                                        continue;
                                                                                                                    }
                                                                                                                }
                                                                                                                else
                                                                                                                if (item7->kind == NODE_KIND_PROPERTY)
                                                                                                                {
                                                                                                                    node_property_t *property2 = (node_property_t *)item7->value;
                                                                                                                    if (semantic_idcmp(property2->key, basic->right) == 1)
                                                                                                                    {
                                                                                                                        if ((clone2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                                                                        {
                                                                                                                            if ((property2->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                                                                            {
                                                                                                                                semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                                                    property2->key->position.path, property2->key->position.line, property2->key->position.column, __FILE__, __LINE__);
                                                                                                                                return -1;
                                                                                                                            }
                                                                                                                        }

                                                                                                                        if ((property2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                                                        {
                                                                                                                            semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                                                property2->key->position.path, property2->key->position.line, property2->key->position.column, __FILE__, __LINE__);
                                                                                                                            return -1;
                                                                                                                        }

                                                                                                                        ilist_t *r6 = list_rpush(response, item7);
                                                                                                                        if (r6 == NULL)
                                                                                                                        {
                                                                                                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                                                            return -1;
                                                                                                                        }
                                                                                                                        continue;
                                                                                                                    }
                                                                                                                }
                                                                                                            }

                                                                                                            ilist_t *r6 = list_rpush(repository1, class2->heritages);
                                                                                                            if (r6 == NULL)
                                                                                                            {
                                                                                                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                                                return -1;
                                                                                                            }

                                                                                                        }
                                                                                                        else
                                                                                                        if (item6->kind == NODE_KIND_GENERIC)
                                                                                                        {
                                                                                                            node5 = item6;
                                                                                                            break;
                                                                                                        }
                                                                                                        else
                                                                                                        {
                                                                                                            semantic_error(program, generic2->type, "Wrong type, in confronting with (%s-%lld:%lld)",
                                                                                                                item6->position.path, item6->position.line, item6->position.column);
                                                                                                            return -1;
                                                                                                        }
                                                                                                    }

                                                                                                    if (cnt_response4 == 0)
                                                                                                    {
                                                                                                        semantic_error(program, generic2->type, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                                                                                        return -1;
                                                                                                    }
                                                                                                }
                                                                                                list_destroy(response5);
                                                                                            }
                                                                                        }
                                                                                    }
                                                                                    else
                                                                                    {
                                                                                        semantic_error(program, heritage1->type, "Wrong type, in confronting with (%s-%lld:%lld)",
                                                                                            item5->position.path, item5->position.line, item5->position.column);
                                                                                        return -1;
                                                                                    }
                                                                                }

                                                                                if (cnt_response4 == 0)
                                                                                {
                                                                                    semantic_error(program, heritage1->type, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                                                                    return -1;
                                                                                }
                                                                            }

                                                                            list_destroy(response4);
                                                                        }
                                                                        else
                                                                        {
                                                                            node_t *node4 = heritage1->value_update;

                                                                            if (node4->kind == NODE_KIND_CLASS)
                                                                            {
                                                                                node_class_t *class2 = (node_class_t *)node4->value;

                                                                                node_t *node5 = class2->block;
                                                                                node_block_t *block3 = (node_block_t *)node5->value;

                                                                                ilist_t *a5;
                                                                                for (a5 = block3->list->begin;a5 != block3->list->end;a5 = a5->next)
                                                                                {
                                                                                    node_t *item4 = (node_t *)a5->value;
                                                                                    if (item4->kind == NODE_KIND_CLASS)
                                                                                    {
                                                                                        node_class_t *class3 = (node_class_t *)item4->value;
                                                                                        if (semantic_idcmp(class3->key, basic->right) == 1)
                                                                                        {
                                                                                            if ((node4->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                                            {
                                                                                                if ((class3->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                                                {
                                                                                                    semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                        class3->key->position.path, class3->key->position.line, class3->key->position.column, __FILE__, __LINE__);
                                                                                                    return -1;
                                                                                                }
                                                                                            }

                                                                                            if ((class3->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                            {
                                                                                                semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                    class3->key->position.path, class3->key->position.line, class3->key->position.column, __FILE__, __LINE__);
                                                                                                return -1;
                                                                                            }

                                                                                            ilist_t *r3 = list_rpush(response, item4);
                                                                                            if (r3 == NULL)
                                                                                            {
                                                                                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                                return -1;
                                                                                            }
                                                                                            continue;
                                                                                        }
                                                                                    }
                                                                                    else
                                                                                    if (item4->kind == NODE_KIND_FUN)
                                                                                    {
                                                                                        node_fun_t *fun1 = (node_fun_t *)item4->value;
                                                                                        if (semantic_idcmp(fun1->key, basic->right) == 1)
                                                                                        {
                                                                                            if ((node4->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                                            {
                                                                                                if ((fun1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                                                {
                                                                                                    semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                        fun1->key->position.path, fun1->key->position.line, fun1->key->position.column, __FILE__, __LINE__);
                                                                                                    return -1;
                                                                                                }
                                                                                            }

                                                                                            if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                            {
                                                                                                semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                    fun1->key->position.path, fun1->key->position.line, fun1->key->position.column, __FILE__, __LINE__);
                                                                                                return -1;
                                                                                            }

                                                                                            ilist_t *r3 = list_rpush(response, item4);
                                                                                            if (r3 == NULL)
                                                                                            {
                                                                                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                                return -1;
                                                                                            }
                                                                                            continue;
                                                                                        }
                                                                                    }
                                                                                    else
                                                                                    if (item4->kind == NODE_KIND_PROPERTY)
                                                                                    {
                                                                                        node_property_t *property2 = (node_property_t *)item4->value;
                                                                                        if (semantic_idcmp(property2->key, basic->right) == 1)
                                                                                        {
                                                                                            if ((node4->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                                            {
                                                                                                if ((property2->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                                                {
                                                                                                    semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                        property2->key->position.path, property2->key->position.line, property2->key->position.column, __FILE__, __LINE__);
                                                                                                    return -1;
                                                                                                }
                                                                                            }

                                                                                            if ((property2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                            {
                                                                                                semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                    property2->key->position.path, property2->key->position.line, property2->key->position.column, __FILE__, __LINE__);
                                                                                                return -1;
                                                                                            }

                                                                                            ilist_t *r3 = list_rpush(response, item4);
                                                                                            if (r3 == NULL)
                                                                                            {
                                                                                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                                return -1;
                                                                                            }
                                                                                            continue;
                                                                                        }
                                                                                    }
                                                                                }

                                                                                ilist_t *r3 = list_rpush(repository1, class2->heritages);
                                                                                if (r3 == NULL)
                                                                                {
                                                                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                    return -1;
                                                                                }
                                                                            }
                                                                        }
                                                                    }
                                                                }
                                                            
                                                                ilist_t *r4 = list_rpop(repository1);
                                                                if (r4 != NULL)
                                                                {
                                                                    node4 = (node_t *)r4->value;
                                                                    continue;
                                                                }
                                                                else
                                                                {
                                                                    node4 = NULL;
                                                                    break;
                                                                }
                                                            }

                                                            list_destroy(repository1);
                                                        }

                                                    }
                                                    else
                                                    if (item3->kind == NODE_KIND_GENERIC)
                                                    {
                                                        node2 = item3;
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
                                    }
                                }
                                else
                                {
                                    semantic_error(program, node1, "Inaccessible\n\tInternal:%s-%u", __FILE__, __LINE__);
                                    return -1;
                                }
                            }

                            if (cnt_response2 == 0)
                            {
                                semantic_error(program, node1, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                return -1;
                            }
                        }
                    }
                }
            }
            else
            if (item1->kind == NODE_KIND_PARAMETER)
            {
                node_parameter_t *parameter1 = (node_parameter_t *)item1->value;
                if (parameter1->value_update != NULL)
                {
                    node_t *value_update1 = parameter1->value_update;
                    if (value_update1->kind == NODE_KIND_CLASS)
                    {
                        int32_t r2 = semantic_cresolve(program, value_update1, response, basic->right, flag);
                        if (r2 == -1)
                        {
                            return -1;
                        }
                    }
                    else
                    {
                        semantic_error(program, basic->left, "Inaccessible\n\tInternal:%s-%u", __FILE__, __LINE__);
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
                            semantic_error(program, basic->left, "Inaccessible, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u", 
                                node1->position.path, node1->position.line, node1->position.column, __FILE__, __LINE__);
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
                                        if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                        {
                                            semantic_error(program, node1, "Instance object, in confronting with (%s-%lld:%lld)",
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

                                        parameter1->value_update = clone1;

                                        int32_t r3 = semantic_cresolve(program, clone1, response, basic->right, flag);
                                        if (r3 == -1)
                                        {
                                            return -1;
                                        }
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
                                                    if ((item3->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                    {
                                                        semantic_error(program, generic1->type, "Instance object, in confronting with (%s-%lld:%lld)",
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

                                                    parameter1->value_update = clone1;

                                                    int32_t r4 = semantic_cresolve(program, clone1, response, basic->right, flag);
                                                    if (r4 == -1)
                                                    {
                                                        return -1;
                                                    }
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
                                    if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        semantic_error(program, node1, "Instance object, in confronting with (%s-%lld:%lld)",
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

                                    parameter1->value_update = clone1;

                                    node_class_t *class1 = (node_class_t *)clone1->value;

                                    node_t *node2 = class1->block;
                                    node_block_t *block1 = (node_block_t *)node2->value;

                                    ilist_t *a3;
                                    for (a3 = block1->list->begin;a3 != block1->list->end;a3 = a3->next)
                                    {
                                        node_t *item3 = (node_t *)a3->value;
                                        if (item3->kind == NODE_KIND_CLASS)
                                        {
                                            node_class_t *class2 = (node_class_t *)item3->value;
                                            if (semantic_idcmp(basic->right, class2->key) == 1)
                                            {
                                                if ((clone1->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                {
                                                    if ((class2->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                    {
                                                        semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                            class2->key->position.path, class2->key->position.line, class2->key->position.column, __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                }

                                                if ((class2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                {
                                                    semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                        class2->key->position.path, class2->key->position.line, class2->key->position.column, __FILE__, __LINE__);
                                                    return -1;
                                                }

                                                ilist_t *r3 = list_rpush(response, item3);
                                                if (r3 == NULL)
                                                {
                                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                continue;
                                            }
                                        }
                                        else
                                        if (item3->kind == NODE_KIND_FUN)
                                        {
                                            node_fun_t *fun2 = (node_fun_t *)item3->value;
                                            if (semantic_idcmp(basic->right, fun2->key) == 1)
                                            {
                                                if ((clone1->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                {
                                                    if ((fun2->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                    {
                                                        semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                            fun2->key->position.path, fun2->key->position.line, fun2->key->position.column, __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                }

                                                if ((fun2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                {
                                                    semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                        fun2->key->position.path, fun2->key->position.line, fun2->key->position.column, __FILE__, __LINE__);
                                                    return -1;
                                                }

                                                ilist_t *r3 = list_rpush(response, item3);
                                                if (r3 == NULL)
                                                {
                                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                continue;
                                            }
                                        }
                                        else
                                        if (item2->kind == NODE_KIND_PROPERTY)
                                        {
                                            node_property_t *property2 = (node_property_t *)item2->value;
                                            if (semantic_idcmp(basic->right, property2->key) == 1)
                                            {
                                                if ((clone1->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                {
                                                    if ((property2->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                    {
                                                        semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                            property2->key->position.path, property2->key->position.line, property2->key->position.column, __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                }

                                                if ((property2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                {
                                                    semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                        property2->key->position.path, property2->key->position.line, property2->key->position.column, __FILE__, __LINE__);
                                                    return -1;
                                                }

                                                ilist_t *r3 = list_rpush(response, item3);
                                                if (r3 == NULL)
                                                {
                                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                continue;
                                            }
                                        }
                                    }

                                    if (class1->heritages != NULL)
                                    {
                                        node_t *node3 = class1->heritages;

                                        list_t *repository1 = list_create();
                                        if (repository1 == NULL)
                                        {
                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }

                                        while (node3 != NULL)
                                        {
                                            node_block_t *block2 = (node_block_t *)node3->value;

                                            ilist_t *a4;
                                            for (a4 = block2->list->begin;a4 != block2->list->end;a4 = a4->next)
                                            {
                                                node_t *item3 = (node_t *)a4->value;
                                                
                                                if (item3->kind == NODE_KIND_HERITAGE)
                                                {
                                                    node_heritage_t *heritage1 = (node_heritage_t *)item3->value;
                                                
                                                    if (heritage1->value_update == NULL)
                                                    {
                                                        list_t *response3 = list_create();
                                                        if (response3 == NULL)
                                                        {
                                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                            return -1;
                                                        }
                                                    
                                                        int32_t r3 = semantic_resolve(program, heritage1->type->parent, heritage1->type, response3, SEMANTIC_FLAG_NONE);
                                                        if (r3 == -1)
                                                        {
                                                            return -1;
                                                        }
                                                        else
                                                        {
                                                            uint64_t cnt_response3 = 0;

                                                            ilist_t *a5;
                                                            for (a5 = response3->begin;a5 != response3->end;a5 = a5->next)
                                                            {
                                                                cnt_response3 += 1;

                                                                node_t *item4 = (node_t *)a5->value;

                                                                if (item4->kind == NODE_KIND_CLASS)
                                                                {
                                                                    if ((item4->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                                    {
                                                                        semantic_error(program, heritage1->type, "Instance object, in confronting with (%s-%lld:%lld)",
                                                                            item4->position.path, item4->position.line, item4->position.column);
                                                                        return -1;
                                                                    }

                                                                    node_t *clone2 = node_clone(item4->parent, item4);
                                                                    if (clone2 == NULL)
                                                                    {
                                                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                        return -1;
                                                                    }
                                                                    clone2->flag |= NODE_FLAG_INSTANCE;

                                                                    heritage1->value_update = clone2;

                                                                    node_class_t *class2 = (node_class_t *)clone2->value;

                                                                    node_t *node4 = class2->block;
                                                                    node_block_t *block3 = (node_block_t *)node4->value;

                                                                    ilist_t *a6;
                                                                    for (a6 = block3->list->begin;a6 != block3->list->end;a6 = a6->next)
                                                                    {
                                                                        node_t *item5 = (node_t *)a6->value;
                                                                        if (item5->kind == NODE_KIND_CLASS)
                                                                        {
                                                                            node_class_t *class3 = (node_class_t *)item5->value;
                                                                            if (semantic_idcmp(class3->key, basic->right) == 1)
                                                                            {
                                                                                if ((clone2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                                {
                                                                                    if ((class3->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                                    {
                                                                                        semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                            class3->key->position.path, class3->key->position.line, class3->key->position.column, __FILE__, __LINE__);
                                                                                        return -1;
                                                                                    }
                                                                                }

                                                                                if ((class3->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                {
                                                                                    semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                        class3->key->position.path, class3->key->position.line, class3->key->position.column, __FILE__, __LINE__);
                                                                                    return -1;
                                                                                }

                                                                                ilist_t *r4 = list_rpush(response, item4);
                                                                                if (r4 == NULL)
                                                                                {
                                                                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                    return -1;
                                                                                }
                                                                                continue;
                                                                            }
                                                                        }
                                                                        else
                                                                        if (item5->kind == NODE_KIND_FUN)
                                                                        {
                                                                            node_fun_t *fun1 = (node_fun_t *)item5->value;
                                                                            if (semantic_idcmp(fun1->key, basic->right) == 1)
                                                                            {
                                                                                if ((clone2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                                {
                                                                                    if ((fun1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                                    {
                                                                                        semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                            fun1->key->position.path, fun1->key->position.line, fun1->key->position.column, __FILE__, __LINE__);
                                                                                        return -1;
                                                                                    }
                                                                                }

                                                                                if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                {
                                                                                    semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                        fun1->key->position.path, fun1->key->position.line, fun1->key->position.column, __FILE__, __LINE__);
                                                                                    return -1;
                                                                                }

                                                                                ilist_t *r4 = list_rpush(response, item4);
                                                                                if (r4 == NULL)
                                                                                {
                                                                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                    return -1;
                                                                                }
                                                                                continue;
                                                                            }
                                                                        }
                                                                        else
                                                                        if (item5->kind == NODE_KIND_PROPERTY)
                                                                        {
                                                                            node_property_t *property2 = (node_property_t *)item5->value;
                                                                            if (semantic_idcmp(property2->key, basic->right) == 1)
                                                                            {
                                                                                if ((clone2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                                {
                                                                                    if ((property2->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                                    {
                                                                                        semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                            property2->key->position.path, property2->key->position.line, property2->key->position.column, __FILE__, __LINE__);
                                                                                        return -1;
                                                                                    }
                                                                                }

                                                                                if ((property2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                {
                                                                                    semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                        property2->key->position.path, property2->key->position.line, property2->key->position.column, __FILE__, __LINE__);
                                                                                    return -1;
                                                                                }

                                                                                ilist_t *r4 = list_rpush(response, item4);
                                                                                if (r4 == NULL)
                                                                                {
                                                                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                    return -1;
                                                                                }
                                                                                continue;
                                                                            }
                                                                        }
                                                                    }

                                                                    ilist_t *r4 = list_rpush(repository1, class2->heritages);
                                                                    if (r4 == NULL)
                                                                    {
                                                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                        return -1;
                                                                    }
                                                                }
                                                                else
                                                                if (item4->kind == NODE_KIND_GENERIC)
                                                                {
                                                                    node_t *node4 = item4;
                                                                    while (node4 != NULL)
                                                                    {
                                                                        node_generic_t *generic1 = (node_generic_t *)node4->value;
                                                                        if (generic1->type != NULL)
                                                                        {
                                                                            list_t *response4 = list_create();
                                                                            if (response4 == NULL)
                                                                            {
                                                                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                return -1;
                                                                            }
                                                                        
                                                                            int32_t r4 = semantic_resolve(program, generic1->type->parent, generic1->type, response4, SEMANTIC_FLAG_NONE);
                                                                            if (r4 == -1)
                                                                            {
                                                                                return -1;
                                                                            }
                                                                            else
                                                                            {
                                                                                uint64_t cnt_response4 = 0;

                                                                                ilist_t *a6;
                                                                                for (a6 = response4->begin;a6 != response4->end;a6 = a6->next)
                                                                                {
                                                                                    cnt_response4 += 1;

                                                                                    node_t *item5 = (node_t *)a6->value;
                                                                                    if (item5->kind == NODE_KIND_CLASS)
                                                                                    {
                                                                                        if ((item5->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                                                        {
                                                                                            semantic_error(program, generic1->type, "Instance object, in confronting with (%s-%lld:%lld)",
                                                                                                item5->position.path, item5->position.line, item5->position.column);
                                                                                            return -1;
                                                                                        }

                                                                                        node_t *clone2 = node_clone(item5->parent, item5);
                                                                                        if (clone2 == NULL)
                                                                                        {
                                                                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                            return -1;
                                                                                        }
                                                                                        clone2->flag |= NODE_FLAG_INSTANCE;

                                                                                        heritage1->value_update = clone2;

                                                                                        node_class_t *class2 = (node_class_t *)clone2->value;

                                                                                        node_t *node5 = class2->block;
                                                                                        node_block_t *block3 = (node_block_t *)node5->value;

                                                                                        ilist_t *a7;
                                                                                        for (a7 = block3->list->begin;a7 != block3->list->end;a7 = a7->next)
                                                                                        {
                                                                                            node_t *item6 = (node_t *)a7->value;
                                                                                            if (item6->kind == NODE_KIND_CLASS)
                                                                                            {
                                                                                                node_class_t *class3 = (node_class_t *)item6->value;
                                                                                                if (semantic_idcmp(class3->key, basic->right) == 1)
                                                                                                {
                                                                                                    if ((clone2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                                                    {
                                                                                                        if ((class3->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                                                        {
                                                                                                            semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                                class3->key->position.path, class3->key->position.line, class3->key->position.column, __FILE__, __LINE__);
                                                                                                            return -1;
                                                                                                        }
                                                                                                    }

                                                                                                    if ((class3->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                                    {
                                                                                                        semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                            class3->key->position.path, class3->key->position.line, class3->key->position.column, __FILE__, __LINE__);
                                                                                                        return -1;
                                                                                                    }

                                                                                                    ilist_t *r5 = list_rpush(response, item6);
                                                                                                    if (r5 == NULL)
                                                                                                    {
                                                                                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                                        return -1;
                                                                                                    }
                                                                                                    continue;
                                                                                                }
                                                                                            }
                                                                                            else
                                                                                            if (item6->kind == NODE_KIND_FUN)
                                                                                            {
                                                                                                node_fun_t *fun1 = (node_fun_t *)item6->value;
                                                                                                if (semantic_idcmp(fun1->key, basic->right) == 1)
                                                                                                {
                                                                                                    if ((clone2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                                                    {
                                                                                                        if ((fun1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                                                        {
                                                                                                            semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                                fun1->key->position.path, fun1->key->position.line, fun1->key->position.column, __FILE__, __LINE__);
                                                                                                            return -1;
                                                                                                        }
                                                                                                    }

                                                                                                    if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                                    {
                                                                                                        semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                            fun1->key->position.path, fun1->key->position.line, fun1->key->position.column, __FILE__, __LINE__);
                                                                                                        return -1;
                                                                                                    }

                                                                                                    ilist_t *r5 = list_rpush(response, item6);
                                                                                                    if (r5 == NULL)
                                                                                                    {
                                                                                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                                        return -1;
                                                                                                    }
                                                                                                    continue;
                                                                                                }
                                                                                            }
                                                                                            else
                                                                                            if (item6->kind == NODE_KIND_PROPERTY)
                                                                                            {
                                                                                                node_property_t *property2 = (node_property_t *)item6->value;
                                                                                                if (semantic_idcmp(property2->key, basic->right) == 1)
                                                                                                {
                                                                                                    if ((clone2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                                                    {
                                                                                                        if ((property2->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                                                        {
                                                                                                            semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                                property2->key->position.path, property2->key->position.line, property2->key->position.column, __FILE__, __LINE__);
                                                                                                            return -1;
                                                                                                        }
                                                                                                    }

                                                                                                    if ((property2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                                    {
                                                                                                        semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                            property2->key->position.path, property2->key->position.line, property2->key->position.column, __FILE__, __LINE__);
                                                                                                        return -1;
                                                                                                    }

                                                                                                    ilist_t *r5 = list_rpush(response, item6);
                                                                                                    if (r5 == NULL)
                                                                                                    {
                                                                                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                                        return -1;
                                                                                                    }
                                                                                                    continue;
                                                                                                }
                                                                                            }
                                                                                        }

                                                                                        ilist_t *r5 = list_rpush(repository1, class2->heritages);
                                                                                        if (r5 == NULL)
                                                                                        {
                                                                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                            return -1;
                                                                                        }

                                                                                    }
                                                                                    else
                                                                                    if (item5->kind == NODE_KIND_GENERIC)
                                                                                    {
                                                                                        node4 = item5;
                                                                                        break;
                                                                                    }
                                                                                    else
                                                                                    {
                                                                                        semantic_error(program, generic1->type, "Wrong type, in confronting with (%s-%lld:%lld)",
                                                                                            item5->position.path, item5->position.line, item5->position.column);
                                                                                        return -1;
                                                                                    }
                                                                                }

                                                                                if (cnt_response4 == 0)
                                                                                {
                                                                                    semantic_error(program, generic1->type, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                                                                    return -1;
                                                                                }
                                                                            }
                                                                            list_destroy(response4);
                                                                        }
                                                                    }
                                                                }
                                                                else
                                                                {
                                                                    semantic_error(program, heritage1->type, "Wrong type, in confronting with (%s-%lld:%lld)",
                                                                        item4->position.path, item4->position.line, item4->position.column);
                                                                    return -1;
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
                                                    else
                                                    {
                                                        node_t *node4 = heritage1->value_update;

                                                        if (node4->kind == NODE_KIND_CLASS)
                                                        {
                                                            node_class_t *class2 = (node_class_t *)node4->value;

                                                            node_t *node5 = class2->block;
                                                            node_block_t *block3 = (node_block_t *)node5->value;

                                                            ilist_t *a5;
                                                            for (a5 = block3->list->begin;a5 != block3->list->end;a5 = a5->next)
                                                            {
                                                                node_t *item4 = (node_t *)a5->value;
                                                                if (item4->kind == NODE_KIND_CLASS)
                                                                {
                                                                    node_class_t *class3 = (node_class_t *)item4->value;
                                                                    if (semantic_idcmp(class3->key, basic->right) == 1)
                                                                    {
                                                                        if ((node4->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                        {
                                                                            if ((class3->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                            {
                                                                                semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                    class3->key->position.path, class3->key->position.line, class3->key->position.column, __FILE__, __LINE__);
                                                                                return -1;
                                                                            }
                                                                        }

                                                                        if ((class3->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                        {
                                                                            semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                class3->key->position.path, class3->key->position.line, class3->key->position.column, __FILE__, __LINE__);
                                                                            return -1;
                                                                        }

                                                                        ilist_t *r3 = list_rpush(response, item4);
                                                                        if (r3 == NULL)
                                                                        {
                                                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                            return -1;
                                                                        }
                                                                        continue;
                                                                    }
                                                                }
                                                                else
                                                                if (item4->kind == NODE_KIND_FUN)
                                                                {
                                                                    node_fun_t *fun1 = (node_fun_t *)item4->value;
                                                                    if (semantic_idcmp(fun1->key, basic->right) == 1)
                                                                    {
                                                                        if ((node4->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                        {
                                                                            if ((fun1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                            {
                                                                                semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                    fun1->key->position.path, fun1->key->position.line, fun1->key->position.column, __FILE__, __LINE__);
                                                                                return -1;
                                                                            }
                                                                        }

                                                                        if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                        {
                                                                            semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                fun1->key->position.path, fun1->key->position.line, fun1->key->position.column, __FILE__, __LINE__);
                                                                            return -1;
                                                                        }

                                                                        ilist_t *r3 = list_rpush(response, item4);
                                                                        if (r3 == NULL)
                                                                        {
                                                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                            return -1;
                                                                        }
                                                                        continue;
                                                                    }
                                                                }
                                                                else
                                                                if (item4->kind == NODE_KIND_PROPERTY)
                                                                {
                                                                    node_property_t *property2 = (node_property_t *)item4->value;
                                                                    if (semantic_idcmp(property2->key, basic->right) == 1)
                                                                    {
                                                                        if ((node4->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                        {
                                                                            if ((property2->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                            {
                                                                                semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                    property2->key->position.path, property2->key->position.line, property2->key->position.column, __FILE__, __LINE__);
                                                                                return -1;
                                                                            }
                                                                        }

                                                                        if ((property2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                        {
                                                                            semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                property2->key->position.path, property2->key->position.line, property2->key->position.column, __FILE__, __LINE__);
                                                                            return -1;
                                                                        }

                                                                        ilist_t *r3 = list_rpush(response, item4);
                                                                        if (r3 == NULL)
                                                                        {
                                                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                            return -1;
                                                                        }
                                                                        continue;
                                                                    }
                                                                }
                                                            }

                                                            ilist_t *r3 = list_rpush(repository1, class2->heritages);
                                                            if (r3 == NULL)
                                                            {
                                                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                return -1;
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        
                                            ilist_t *r3 = list_rpop(repository1);
                                            if (r3 != NULL)
                                            {
                                                node3 = (node_t *)r3->value;
                                                continue;
                                            }
                                            else
                                            {
                                                node3 = NULL;
                                                break;
                                            }
                                        }

                                        list_destroy(repository1);
                                    }
                                }
                                else
                                if (item2->kind == NODE_KIND_GENERIC)
                                {
                                    node_t *node2 = item2;
                                    while (node2 != NULL)
                                    {
                                        node_generic_t *generic1 = (node_generic_t *)node2->value;

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
                                                        if ((item3->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                        {
                                                            semantic_error(program, generic1->type, "Instance object, in confronting with (%s-%lld:%lld)",
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

                                                        parameter1->value_update = clone1;

                                                        node_class_t *class1 = (node_class_t *)clone1->value;

                                                        node_t *node3 = class1->block;
                                                        node_block_t *block1 = (node_block_t *)node3->value;

                                                        ilist_t *a4;
                                                        for (a4 = block1->list->begin;a4 != block1->list->end;a4 = a4->next)
                                                        {
                                                            node_t *item4 = (node_t *)a4->value;
                                                            if (item4->kind == NODE_KIND_CLASS)
                                                            {
                                                                node_class_t *class2 = (node_class_t *)item4->value;
                                                                if (semantic_idcmp(basic->right, class2->key) == 1)
                                                                {
                                                                    if ((clone1->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                    {
                                                                        if ((class2->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                        {
                                                                            semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                class2->key->position.path, class2->key->position.line, class2->key->position.column, __FILE__, __LINE__);
                                                                            return -1;
                                                                        }
                                                                    }

                                                                    if ((class2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                    {
                                                                        semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                            class2->key->position.path, class2->key->position.line, class2->key->position.column, __FILE__, __LINE__);
                                                                        return -1;
                                                                    }

                                                                    ilist_t *r4 = list_rpush(response, item4);
                                                                    if (r4 == NULL)
                                                                    {
                                                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                        return -1;
                                                                    }
                                                                    continue;
                                                                }
                                                            }
                                                            else
                                                            if (item4->kind == NODE_KIND_FUN)
                                                            {
                                                                node_fun_t *fun2 = (node_fun_t *)item4->value;
                                                                if (semantic_idcmp(basic->right, fun2->key) == 1)
                                                                {
                                                                    if ((clone1->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                    {
                                                                        if ((fun2->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                        {
                                                                            semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                fun2->key->position.path, fun2->key->position.line, fun2->key->position.column, __FILE__, __LINE__);
                                                                            return -1;
                                                                        }
                                                                    }

                                                                    if ((fun2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                    {
                                                                        semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                            fun2->key->position.path, fun2->key->position.line, fun2->key->position.column, __FILE__, __LINE__);
                                                                        return -1;
                                                                    }

                                                                    ilist_t *r4 = list_rpush(response, item4);
                                                                    if (r4 == NULL)
                                                                    {
                                                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                        return -1;
                                                                    }
                                                                    continue;
                                                                }
                                                            }
                                                            else
                                                            if (item4->kind == NODE_KIND_PROPERTY)
                                                            {
                                                                node_property_t *property2 = (node_property_t *)item4->value;
                                                                if (semantic_idcmp(basic->right, property2->key) == 1)
                                                                {
                                                                    if ((clone1->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                    {
                                                                        if ((property2->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                        {
                                                                            semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                property2->key->position.path, property2->key->position.line, property2->key->position.column, __FILE__, __LINE__);
                                                                            return -1;
                                                                        }
                                                                    }

                                                                    if ((property2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                    {
                                                                        semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                            property2->key->position.path, property2->key->position.line, property2->key->position.column, __FILE__, __LINE__);
                                                                        return -1;
                                                                    }

                                                                    ilist_t *r4 = list_rpush(response, item4);
                                                                    if (r4 == NULL)
                                                                    {
                                                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                        return -1;
                                                                    }
                                                                    continue;
                                                                }
                                                            }
                                                        }

                                                        if (class1->heritages != NULL)
                                                        {
                                                            node_t *node4 = class1->heritages;

                                                            list_t *repository1 = list_create();
                                                            if (repository1 == NULL)
                                                            {
                                                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                return -1;
                                                            }

                                                            while (node4 != NULL)
                                                            {
                                                                node_block_t *block2 = (node_block_t *)node4->value;

                                                                ilist_t *a5;
                                                                for (a5 = block2->list->begin;a5 != block2->list->end;a5 = a5->next)
                                                                {
                                                                    node_t *item4 = (node_t *)a5->value;
                                                                    
                                                                    if (item4->kind == NODE_KIND_HERITAGE)
                                                                    {
                                                                        node_heritage_t *heritage1 = (node_heritage_t *)item4->value;
                                                                    
                                                                        if (heritage1->value_update == NULL)
                                                                        {
                                                                            list_t *response4 = list_create();
                                                                            if (response4 == NULL)
                                                                            {
                                                                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                return -1;
                                                                            }
                                                                        
                                                                            int32_t r4 = semantic_resolve(program, heritage1->type->parent, heritage1->type, response4, SEMANTIC_FLAG_NONE);
                                                                            if (r4 == -1)
                                                                            {
                                                                                return -1;
                                                                            }
                                                                            else
                                                                            {
                                                                                uint64_t cnt_response4 = 0;

                                                                                ilist_t *a6;
                                                                                for (a6 = response4->begin;a6 != response4->end;a6 = a6->next)
                                                                                {
                                                                                    cnt_response4 += 1;

                                                                                    node_t *item5 = (node_t *)a6->value;

                                                                                    if (item5->kind == NODE_KIND_CLASS)
                                                                                    {
                                                                                        if ((item5->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                                                        {
                                                                                            semantic_error(program, heritage1->type, "Instance object, in confronting with (%s-%lld:%lld)",
                                                                                                item5->position.path, item5->position.line, item5->position.column);
                                                                                            return -1;
                                                                                        }

                                                                                        node_t *clone2 = node_clone(item5->parent, item5);
                                                                                        if (clone2 == NULL)
                                                                                        {
                                                                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                            return -1;
                                                                                        }
                                                                                        clone2->flag |= NODE_FLAG_INSTANCE;

                                                                                        heritage1->value_update = clone2;

                                                                                        node_class_t *class2 = (node_class_t *)clone2->value;

                                                                                        node_t *node5 = class2->block;
                                                                                        node_block_t *block3 = (node_block_t *)node5->value;

                                                                                        ilist_t *a7;
                                                                                        for (a7 = block3->list->begin;a7 != block3->list->end;a7 = a7->next)
                                                                                        {
                                                                                            node_t *item6 = (node_t *)a7->value;
                                                                                            if (item6->kind == NODE_KIND_CLASS)
                                                                                            {
                                                                                                node_class_t *class3 = (node_class_t *)item6->value;
                                                                                                if (semantic_idcmp(class3->key, basic->right) == 1)
                                                                                                {
                                                                                                    if ((clone2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                                                    {
                                                                                                        if ((class3->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                                                        {
                                                                                                            semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                                class3->key->position.path, class3->key->position.line, class3->key->position.column, __FILE__, __LINE__);
                                                                                                            return -1;
                                                                                                        }
                                                                                                    }

                                                                                                    if ((class3->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                                    {
                                                                                                        semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                            class3->key->position.path, class3->key->position.line, class3->key->position.column, __FILE__, __LINE__);
                                                                                                        return -1;
                                                                                                    }

                                                                                                    ilist_t *r5 = list_rpush(response, item6);
                                                                                                    if (r5 == NULL)
                                                                                                    {
                                                                                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                                        return -1;
                                                                                                    }
                                                                                                    continue;
                                                                                                }
                                                                                            }
                                                                                            else
                                                                                            if (item6->kind == NODE_KIND_FUN)
                                                                                            {
                                                                                                node_fun_t *fun1 = (node_fun_t *)item6->value;
                                                                                                if (semantic_idcmp(fun1->key, basic->right) == 1)
                                                                                                {
                                                                                                    if ((clone2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                                                    {
                                                                                                        if ((fun1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                                                        {
                                                                                                            semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                                fun1->key->position.path, fun1->key->position.line, fun1->key->position.column, __FILE__, __LINE__);
                                                                                                            return -1;
                                                                                                        }
                                                                                                    }

                                                                                                    if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                                    {
                                                                                                        semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                            fun1->key->position.path, fun1->key->position.line, fun1->key->position.column, __FILE__, __LINE__);
                                                                                                        return -1;
                                                                                                    }

                                                                                                    ilist_t *r5 = list_rpush(response, item6);
                                                                                                    if (r5 == NULL)
                                                                                                    {
                                                                                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                                        return -1;
                                                                                                    }
                                                                                                    continue;
                                                                                                }
                                                                                            }
                                                                                            else
                                                                                            if (item6->kind == NODE_KIND_PROPERTY)
                                                                                            {
                                                                                                node_property_t *property2 = (node_property_t *)item6->value;
                                                                                                if (semantic_idcmp(property2->key, basic->right) == 1)
                                                                                                {
                                                                                                    if ((clone2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                                                    {
                                                                                                        if ((property2->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                                                        {
                                                                                                            semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                                property2->key->position.path, property2->key->position.line, property2->key->position.column, __FILE__, __LINE__);
                                                                                                            return -1;
                                                                                                        }
                                                                                                    }

                                                                                                    if ((property2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                                    {
                                                                                                        semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                            property2->key->position.path, property2->key->position.line, property2->key->position.column, __FILE__, __LINE__);
                                                                                                        return -1;
                                                                                                    }

                                                                                                    ilist_t *r5 = list_rpush(response, item6);
                                                                                                    if (r5 == NULL)
                                                                                                    {
                                                                                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                                        return -1;
                                                                                                    }
                                                                                                    continue;
                                                                                                }
                                                                                            }
                                                                                        }

                                                                                        ilist_t *r5 = list_rpush(repository1, class2->heritages);
                                                                                        if (r5 == NULL)
                                                                                        {
                                                                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                            return -1;
                                                                                        }
                                                                                    }
                                                                                    else
                                                                                    if (item5->kind == NODE_KIND_GENERIC)
                                                                                    {
                                                                                        node_t *node5 = item5;
                                                                                        while (node5 != NULL)
                                                                                        {
                                                                                            node_generic_t *generic2 = (node_generic_t *)node5->value;
                                                                                            if (generic2->type != NULL)
                                                                                            {
                                                                                                list_t *response5 = list_create();
                                                                                                if (response5 == NULL)
                                                                                                {
                                                                                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                                    return -1;
                                                                                                }
                                                                                            
                                                                                                int32_t r5 = semantic_resolve(program, generic2->type->parent, generic2->type, response5, SEMANTIC_FLAG_NONE);
                                                                                                if (r5 == -1)
                                                                                                {
                                                                                                    return -1;
                                                                                                }
                                                                                                else
                                                                                                {
                                                                                                    uint64_t cnt_response5 = 0;

                                                                                                    ilist_t *a7;
                                                                                                    for (a7 = response5->begin;a7 != response5->end;a7 = a7->next)
                                                                                                    {
                                                                                                        cnt_response5 += 1;

                                                                                                        node_t *item6 = (node_t *)a7->value;
                                                                                                        if (item6->kind == NODE_KIND_CLASS)
                                                                                                        {
                                                                                                            if ((item6->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                                                                            {
                                                                                                                semantic_error(program, generic1->type, "Instance object, in confronting with (%s-%lld:%lld)",
                                                                                                                    item6->position.path, item6->position.line, item6->position.column);
                                                                                                                return -1;
                                                                                                            }

                                                                                                            node_t *clone2 = node_clone(item6->parent, item6);
                                                                                                            if (clone2 == NULL)
                                                                                                            {
                                                                                                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                                                return -1;
                                                                                                            }
                                                                                                            clone2->flag |= NODE_FLAG_INSTANCE;

                                                                                                            heritage1->value_update = clone2;

                                                                                                            node_class_t *class2 = (node_class_t *)clone2->value;

                                                                                                            node_t *node6 = class2->block;
                                                                                                            node_block_t *block3 = (node_block_t *)node6->value;

                                                                                                            ilist_t *a8;
                                                                                                            for (a8 = block3->list->begin;a8 != block3->list->end;a8 = a8->next)
                                                                                                            {
                                                                                                                node_t *item7 = (node_t *)a8->value;
                                                                                                                if (item7->kind == NODE_KIND_CLASS)
                                                                                                                {
                                                                                                                    node_class_t *class3 = (node_class_t *)item7->value;
                                                                                                                    if (semantic_idcmp(class3->key, basic->right) == 1)
                                                                                                                    {
                                                                                                                        if ((clone2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                                                                        {
                                                                                                                            if ((class3->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                                                                            {
                                                                                                                                semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                                                    class3->key->position.path, class3->key->position.line, class3->key->position.column, __FILE__, __LINE__);
                                                                                                                                return -1;
                                                                                                                            }
                                                                                                                        }

                                                                                                                        if ((class3->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                                                        {
                                                                                                                            semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                                                class3->key->position.path, class3->key->position.line, class3->key->position.column, __FILE__, __LINE__);
                                                                                                                            return -1;
                                                                                                                        }

                                                                                                                        ilist_t *r6 = list_rpush(response, item7);
                                                                                                                        if (r6 == NULL)
                                                                                                                        {
                                                                                                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                                                            return -1;
                                                                                                                        }
                                                                                                                        continue;
                                                                                                                    }
                                                                                                                }
                                                                                                                else
                                                                                                                if (item7->kind == NODE_KIND_FUN)
                                                                                                                {
                                                                                                                    node_fun_t *fun1 = (node_fun_t *)item7->value;
                                                                                                                    if (semantic_idcmp(fun1->key, basic->right) == 1)
                                                                                                                    {
                                                                                                                        if ((clone2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                                                                        {
                                                                                                                            if ((fun1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                                                                            {
                                                                                                                                semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                                                    fun1->key->position.path, fun1->key->position.line, fun1->key->position.column, __FILE__, __LINE__);
                                                                                                                                return -1;
                                                                                                                            }
                                                                                                                        }

                                                                                                                        if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                                                        {
                                                                                                                            semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                                                fun1->key->position.path, fun1->key->position.line, fun1->key->position.column, __FILE__, __LINE__);
                                                                                                                            return -1;
                                                                                                                        }

                                                                                                                        ilist_t *r6 = list_rpush(response, item7);
                                                                                                                        if (r6 == NULL)
                                                                                                                        {
                                                                                                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                                                            return -1;
                                                                                                                        }
                                                                                                                        continue;
                                                                                                                    }
                                                                                                                }
                                                                                                                else
                                                                                                                if (item7->kind == NODE_KIND_PROPERTY)
                                                                                                                {
                                                                                                                    node_property_t *property2 = (node_property_t *)item7->value;
                                                                                                                    if (semantic_idcmp(property2->key, basic->right) == 1)
                                                                                                                    {
                                                                                                                        if ((clone2->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                                                                        {
                                                                                                                            if ((property2->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                                                                            {
                                                                                                                                semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                                                    property2->key->position.path, property2->key->position.line, property2->key->position.column, __FILE__, __LINE__);
                                                                                                                                return -1;
                                                                                                                            }
                                                                                                                        }

                                                                                                                        if ((property2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                                                        {
                                                                                                                            semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                                                property2->key->position.path, property2->key->position.line, property2->key->position.column, __FILE__, __LINE__);
                                                                                                                            return -1;
                                                                                                                        }

                                                                                                                        ilist_t *r6 = list_rpush(response, item7);
                                                                                                                        if (r6 == NULL)
                                                                                                                        {
                                                                                                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                                                            return -1;
                                                                                                                        }
                                                                                                                        continue;
                                                                                                                    }
                                                                                                                }
                                                                                                            }

                                                                                                            ilist_t *r6 = list_rpush(repository1, class2->heritages);
                                                                                                            if (r6 == NULL)
                                                                                                            {
                                                                                                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                                                return -1;
                                                                                                            }

                                                                                                        }
                                                                                                        else
                                                                                                        if (item6->kind == NODE_KIND_GENERIC)
                                                                                                        {
                                                                                                            node5 = item6;
                                                                                                            break;
                                                                                                        }
                                                                                                        else
                                                                                                        {
                                                                                                            semantic_error(program, generic2->type, "Wrong type, in confronting with (%s-%lld:%lld)",
                                                                                                                item6->position.path, item6->position.line, item6->position.column);
                                                                                                            return -1;
                                                                                                        }
                                                                                                    }

                                                                                                    if (cnt_response4 == 0)
                                                                                                    {
                                                                                                        semantic_error(program, generic2->type, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                                                                                        return -1;
                                                                                                    }
                                                                                                }
                                                                                                list_destroy(response5);
                                                                                            }
                                                                                        }
                                                                                    }
                                                                                    else
                                                                                    {
                                                                                        semantic_error(program, heritage1->type, "Wrong type, in confronting with (%s-%lld:%lld)",
                                                                                            item5->position.path, item5->position.line, item5->position.column);
                                                                                        return -1;
                                                                                    }
                                                                                }

                                                                                if (cnt_response4 == 0)
                                                                                {
                                                                                    semantic_error(program, heritage1->type, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                                                                    return -1;
                                                                                }
                                                                            }

                                                                            list_destroy(response4);
                                                                        }
                                                                        else
                                                                        {
                                                                            node_t *node4 = heritage1->value_update;

                                                                            if (node4->kind == NODE_KIND_CLASS)
                                                                            {
                                                                                node_class_t *class2 = (node_class_t *)node4->value;

                                                                                node_t *node5 = class2->block;
                                                                                node_block_t *block3 = (node_block_t *)node5->value;

                                                                                ilist_t *a5;
                                                                                for (a5 = block3->list->begin;a5 != block3->list->end;a5 = a5->next)
                                                                                {
                                                                                    node_t *item4 = (node_t *)a5->value;
                                                                                    if (item4->kind == NODE_KIND_CLASS)
                                                                                    {
                                                                                        node_class_t *class3 = (node_class_t *)item4->value;
                                                                                        if (semantic_idcmp(class3->key, basic->right) == 1)
                                                                                        {
                                                                                            if ((node4->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                                            {
                                                                                                if ((class3->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                                                {
                                                                                                    semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                        class3->key->position.path, class3->key->position.line, class3->key->position.column, __FILE__, __LINE__);
                                                                                                    return -1;
                                                                                                }
                                                                                            }

                                                                                            if ((class3->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                            {
                                                                                                semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                    class3->key->position.path, class3->key->position.line, class3->key->position.column, __FILE__, __LINE__);
                                                                                                return -1;
                                                                                            }

                                                                                            ilist_t *r3 = list_rpush(response, item4);
                                                                                            if (r3 == NULL)
                                                                                            {
                                                                                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                                return -1;
                                                                                            }
                                                                                            continue;
                                                                                        }
                                                                                    }
                                                                                    else
                                                                                    if (item4->kind == NODE_KIND_FUN)
                                                                                    {
                                                                                        node_fun_t *fun1 = (node_fun_t *)item4->value;
                                                                                        if (semantic_idcmp(fun1->key, basic->right) == 1)
                                                                                        {
                                                                                            if ((node4->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                                            {
                                                                                                if ((fun1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                                                {
                                                                                                    semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                        fun1->key->position.path, fun1->key->position.line, fun1->key->position.column, __FILE__, __LINE__);
                                                                                                    return -1;
                                                                                                }
                                                                                            }

                                                                                            if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                            {
                                                                                                semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                    fun1->key->position.path, fun1->key->position.line, fun1->key->position.column, __FILE__, __LINE__);
                                                                                                return -1;
                                                                                            }

                                                                                            ilist_t *r3 = list_rpush(response, item4);
                                                                                            if (r3 == NULL)
                                                                                            {
                                                                                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                                return -1;
                                                                                            }
                                                                                            continue;
                                                                                        }
                                                                                    }
                                                                                    else
                                                                                    if (item4->kind == NODE_KIND_PROPERTY)
                                                                                    {
                                                                                        node_property_t *property2 = (node_property_t *)item4->value;
                                                                                        if (semantic_idcmp(property2->key, basic->right) == 1)
                                                                                        {
                                                                                            if ((node4->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                                                            {
                                                                                                if ((property2->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                                                                                                {
                                                                                                    semantic_error(program, basic->right, "Non-Static, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                        property2->key->position.path, property2->key->position.line, property2->key->position.column, __FILE__, __LINE__);
                                                                                                    return -1;
                                                                                                }
                                                                                            }

                                                                                            if ((property2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                                                                            {
                                                                                                semantic_error(program, basic->right, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                                                                                    property2->key->position.path, property2->key->position.line, property2->key->position.column, __FILE__, __LINE__);
                                                                                                return -1;
                                                                                            }

                                                                                            ilist_t *r3 = list_rpush(response, item4);
                                                                                            if (r3 == NULL)
                                                                                            {
                                                                                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                                return -1;
                                                                                            }
                                                                                            continue;
                                                                                        }
                                                                                    }
                                                                                }

                                                                                ilist_t *r3 = list_rpush(repository1, class2->heritages);
                                                                                if (r3 == NULL)
                                                                                {
                                                                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                                                    return -1;
                                                                                }
                                                                            }
                                                                        }
                                                                    }
                                                                }
                                                            
                                                                ilist_t *r4 = list_rpop(repository1);
                                                                if (r4 != NULL)
                                                                {
                                                                    node4 = (node_t *)r4->value;
                                                                    continue;
                                                                }
                                                                else
                                                                {
                                                                    node4 = NULL;
                                                                    break;
                                                                }
                                                            }

                                                            list_destroy(repository1);
                                                        }

                                                    }
                                                    else
                                                    if (item3->kind == NODE_KIND_GENERIC)
                                                    {
                                                        node2 = item3;
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
                                    }
                                }
                                else
                                {
                                    semantic_error(program, node1, "Inaccessible\n\tInternal:%s-%u", __FILE__, __LINE__);
                                    return -1;
                                }
                            }

                            if (cnt_response2 == 0)
                            {
                                semantic_error(program, node1, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                return -1;
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
                    node_t *value_update1 = heritage1->value_update;
                    if (value_update1->kind == NODE_KIND_CLASS)
                    {
                        int32_t r2 = semantic_cresolve(program, value_update1, response, basic->right, flag);
                        if (r2 == -1)
                        {
                            return -1;
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
                
                    int32_t r2 = semantic_resolve(program, heritage1->type->parent, heritage1->type, response2, SEMANTIC_FLAG_NONE);
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
                                    semantic_error(program, heritage1->type, "Instance object, in confronting with (%s-%lld:%lld)",
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

                                heritage1->value_update = clone1;

                                int32_t r3 = semantic_cresolve(program, clone1, response, basic->right, flag);
                                if (r3 == -1)
                                {
                                    return -1;
                                }
                            }
                            else
                            {
                                semantic_error(program, item2, "Wrong type, for (%lld:%lld)",
                                    heritage1->type->position.line, heritage1->type->position.column);
                                return -1;
                            }
                            break;
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
				semantic_error(program, basic->left, "Inaccessible\n\tInternal:%s-%u", __FILE__, __LINE__);
				return -1;
			}
        }

        if (cnt_response1 == 0)
        {
            semantic_error(program, basic->left, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
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
