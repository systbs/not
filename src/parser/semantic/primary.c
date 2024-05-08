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
semantic_select(program_t *program, node_t *base, node_t *name, list_t *response, node_t *origin_class, uint16_t follow)
{
    if (base->kind == NODE_KIND_CATCH)
    {
        node_catch_t *catch1 = (node_catch_t *)base->value;

        node_t *node1 = catch1->parameters;
        node_block_t *block1 = (node_block_t *)node1->value;

        ilist_t *a1;
        for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
        {
            node_t *item1 = (node_t *)a1->value;
            if (item1->kind == NODE_KIND_PARAMETER)
            {
                node_parameter_t *parameter1 = (node_parameter_t *)item1->value;
                if (semantic_idcmp(parameter1->key, name) == 1)
                {
                    ilist_t *r1 = list_rpush(response, item1);
                    if (r1 == NULL)
                    {
                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                    return 1;
                }
            }
        }
    }
    else
    if (base->kind == NODE_KIND_FOR)
    {
        node_for_t *for1 = (node_for_t *)base->value;

        node_t *node1 = for1->initializer;
        node_block_t *block1 = (node_block_t *)node1->value;

        ilist_t *a1;
        for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
        {
            node_t *item1 = (node_t *)a1->value;
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
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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
                            node_entity_t *entity1 = (node_entity_t *)item2->value;
                            if (semantic_idcmp(entity1->key, name) == 1)
                            {
                                ilist_t *r1 = list_rpush(response, item2);
                                if (r1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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
    if (base->kind == NODE_KIND_BODY)
    {
        node_block_t *block1 = (node_block_t *)base->value;
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
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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
                            node_entity_t *entity1 = (node_entity_t *)item2->value;
                            if (semantic_idcmp(entity1->key, name) == 1)
                            {
                                ilist_t *r1 = list_rpush(response, item2);
                                if (r1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
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
    if (base->kind == NODE_KIND_LAMBDA)
    {
        node_lambda_t *fun1 = (node_lambda_t *)base->value;

        if (fun1->generics != NULL)
        {
            node_t *node1 = fun1->generics;
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
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }
                        return 1;
                    }
                }
            }
        }

        if (fun1->parameters != NULL)
        {
            node_t *node1 = fun1->parameters;
            node_block_t *block1 = (node_block_t *)node1->value;

            ilist_t *a1;
            for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;

                if (item1->kind == NODE_KIND_PARAMETER)
                {
                    node_parameter_t *parameter1 = (node_parameter_t *)item1->value;
                    if (semantic_idcmp(parameter1->key, name) == 1)
                    {
                        ilist_t *r1 = list_rpush(response, item1);
                        if (r1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }
                        return 1;
                    }
                }
            }
        }
    }
    else
    if (base->kind == NODE_KIND_FUN)
    {
        node_fun_t *fun1 = (node_fun_t *)base->value;

        if (fun1->generics != NULL)
        {
            node_t *node1 = fun1->generics;
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
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }
                        return 1;
                    }
                }
            }
        }

        if (fun1->parameters != NULL)
        {
            node_t *node1 = fun1->parameters;
            node_block_t *block1 = (node_block_t *)node1->value;

            ilist_t *a1;
            for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;

                if (item1->kind == NODE_KIND_PARAMETER)
                {
                    node_parameter_t *parameter1 = (node_parameter_t *)item1->value;
                    if (semantic_idcmp(parameter1->key, name) == 1)
                    {
                        ilist_t *r1 = list_rpush(response, item1);
                        if (r1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }
                        return 1;
                    }
                }
            }
        }
    }
    else
    if (base->kind == NODE_KIND_PACKAGE)
    {
        node_package_t *package1 = (node_package_t *)base->value;
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
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }
                        return 1;
                    }
                }
            }
        }
    }
    else
    if (base->kind == NODE_KIND_CLASS)
    {
        node_class_t *class1 = (node_class_t *)base->value;

        if (class1->generics != NULL)
        {
            node_t *node1 = class1->generics;
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
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }
                        return 1;
                    }
                }
            }
        }

        if (class1->heritages != NULL)
        {
            node_t *node1 = class1->heritages;
            node_block_t *block1 = (node_block_t *)node1->value;

            ilist_t *a1;
            for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;

                if (item1->kind == NODE_KIND_HERITAGE)
                {
                    node_heritage_t *heritage1 = (node_heritage_t *)item1->value;
                    if (semantic_idcmp(heritage1->key, name) == 1)
                    {
                        ilist_t *r1 = list_rpush(response, item1);
                        if (r1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }
                        return 1;
                    }
                }
            }
        }

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
                    if (origin_class != NULL)
                    {
                        if ((class2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                        {
                            semantic_error(program, name, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                class2->key->position.path, class2->key->position.line, class2->key->position.column, __FILE__, __LINE__);
                            return -1;
                        }
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
            else
            if (item1->kind == NODE_KIND_FUN)
            {
                node_fun_t *fun1 = (node_fun_t *)item1->value;
                if (semantic_idcmp(fun1->key, name) == 1)
                {
                    if (origin_class != NULL)
                    {
                        if ((fun1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                        {
                            continue;
                        }
                    }

                    ilist_t *r1 = list_rpush(response, item1);
                    if (r1 == NULL)
                    {
                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }      
                }
                continue;
            }
            else
            if (item1->kind == NODE_KIND_PROPERTY)
            {
                node_property_t *property1 = (node_property_t *)item1->value;
                if (semantic_idcmp(property1->key, name) == 1)
                {
                    if (origin_class != NULL)
                    {
                        if ((property1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                        {
                            semantic_error(program, name, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                property1->key->position.path, property1->key->position.line, property1->key->position.column, __FILE__, __LINE__);
                            return -1;
                        }
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

                    if (name->id == heritage3->type->id)
                    {
                        continue;
                    }

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
                                        semantic_error(program, heritage3->type, "Instance object, in confronting with (%s-%lld:%lld)",
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
                                    int32_t r2 = semantic_select(program, clone1, name, response, origin_class, 1);
                                    if (r2 == -1)
                                    {
                                        return -1;
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

                                                    int32_t r2 = semantic_select(program, clone1, name, response, origin_class, 1);
                                                    if (r2 == -1)
                                                    {
                                                        return -1;
                                                    }
                                                }
                                            }

                                            if (cnt_response3 == 0)
                                            {
                                                node_t *node3 = generic1->key;
                                                node_basic_t *basic1 = (node_basic_t *)node3->value;

                                                semantic_error(program, generic1->key, "Reference:type of '%s' not found\n\tInternal:%s-%u", 
                                                    basic1->value, __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }

                                        list_destroy(response3);
                                    }
                                }
                                else
                                {
                                    node_t *node3 = heritage3->key;
                                    node_basic_t *basic1 = (node_basic_t *)node3->value;

                                    semantic_error(program, heritage3->type, "Typing:type of '%s' not valid\n\tInternal:%s-%u",
                                        basic1->value, __FILE__, __LINE__);
                                    return -1;
                                }
                                break;
                            }

                            if (cnt_response2 == 0)
                            {
                                node_t *node3 = heritage3->key;
                                node_basic_t *basic1 = (node_basic_t *)node3->value;

                                semantic_error(program, heritage3->type, "Reference:type of '%s' not found\n\tInternal:%s-%u", 
                                    basic1->value, __FILE__, __LINE__);
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
                            int32_t r2 = semantic_select(program, value_update1, name, response, origin_class, 1);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                        }
                    }
                }
            }
        }

        if (list_count(response) > 0)
        {
            return 1;
        }
    }
    else
    if (base->kind == NODE_KIND_MODULE)
    {
        node_module_t *module1 = (node_module_t *)base->value;

        ilist_t *a1;
        for (a1 = module1->items->begin;a1 != module1->items->end;a1 = a1->next)
        {
            node_t *item1 = (node_t *)a1->value;
            if (item1->kind == NODE_KIND_USING)
            {
                node_t *base2 = name->parent;
                while (base2 != NULL)
                {
                    if (base2->kind == NODE_KIND_MODULE)
                    {
                        break;
                    }
                    base2 = base2->parent;
                }

                if ((base2 != NULL) && (base->id != base2->id))
                {
                    continue;
                }

                node_using_t *using1 = (node_using_t *)item1->value;

                if (using1->packages != NULL)
                {
                    node_t *node1 = using1->packages;
                    node_block_t *block1 = (node_block_t *)node1->value;

                    ilist_t *a2;
                    for (a2 = block1->list->begin;a2 != block1->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_PACKAGE)
                        {
                            node_package_t *package1 = (node_package_t *)item2->value;
                            if (semantic_idcmp(package1->key, name) == 1)
                            {
                                ilist_t *r1 = list_rpush(response, item2);
                                if (r1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                return 1;
                            }
                        }
                    }
                }
                else
                {
                    node_basic_t *basic2 = (node_basic_t *)using1->path->value;

                    node_t *node1 = program_load(program, basic2->value);
                    if (node1 == NULL)
                    {
                        return -1;
                    }

                    node_module_t *module2 = (node_module_t *)node1->value;

                    ilist_t *a2;
                    for (a2 = module2->items->begin;a2 != module2->items->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;

                        if (item2->kind == NODE_KIND_CLASS)
                        {
                            node_class_t *class2 = (node_class_t *)item2->value;
                            if (semantic_idcmp(class2->key, name) == 1)
                            {
                                if ((class2->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                                {
                                    semantic_error(program, name, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                        class2->key->position.path, class2->key->position.line, class2->key->position.column, __FILE__, __LINE__);
                                    return -1;
                                }

                                ilist_t *r1 = list_rpush(response, item2);
                                if (r1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                return 1;
                            }
                            continue;
                        }
                    }
                }
                continue;
            }
            else
            if (item1->kind == NODE_KIND_CLASS)
            {
                node_class_t *class1 = (node_class_t *)item1->value;
                if (semantic_idcmp(class1->key, name) == 1)
                {
                    node_t *base2 = name->parent;
                    while (base2 != NULL)
                    {
                        if (base2->kind == NODE_KIND_MODULE)
                        {
                            break;
                        }
                        base2 = base2->parent;
                    }

                    if ((origin_class != NULL) || (base->id != base2->id))
                    {
                        if ((class1->flag & SYNTAX_MODIFIER_EXPORT) != SYNTAX_MODIFIER_EXPORT)
                        {
                            semantic_error(program, name, "Private access, in confronting with (%s-%lld:%lld)\n\tInternal:%s-%u",
                                class1->key->position.path, class1->key->position.line, class1->key->position.column, __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    ilist_t *r1 = list_rpush(response, item1);
                    if (r1 == NULL)
                    {
                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                    return 1;
                }
                continue;
            }
        }
    }
    
    if ((base->parent != NULL) && (follow != 1))
    {
        if ((base->kind == NODE_KIND_CLASS) && (origin_class == NULL))
        {
            return semantic_select(program, base->parent, name, response, base, follow);
        }
        else
        {
            return semantic_select(program, base->parent, name, response, origin_class, follow);
        }
    }

    return 0;
}

static int32_t
semantic_id(program_t *program, node_t *base, node_t *node, list_t *response, uint64_t flag)
{
    //node_basic_t *basic1 = (node_basic_t *)node->value;
    //printf("semantic %s\n", basic1->value);
	return semantic_select(program, base, node, response, NULL, 0);
}


static int32_t
semantic_int8(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    node_t *clone1 = node_clone(node->parent, node);
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
    return 1;
}

static int32_t
semantic_int16(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    node_t *clone1 = node_clone(node->parent, node);
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
    return 1;
}

static int32_t
semantic_int32(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    node_t *clone1 = node_clone(node->parent, node);
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
    return 1;
}

static int32_t
semantic_int64(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    node_t *clone1 = node_clone(node->parent, node);
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
    return 1;
}


static int32_t
semantic_uint8(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    node_t *clone1 = node_clone(node->parent, node);
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
    return 1;
}

static int32_t
semantic_uint16(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    node_t *clone1 = node_clone(node->parent, node);
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
    return 1;
}

static int32_t
semantic_uint32(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    node_t *clone1 = node_clone(node->parent, node);
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
    return 1;
}

static int32_t
semantic_uint64(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    node_t *clone1 = node_clone(node->parent, node);
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
    return 1;
}

static int32_t
semantic_bigint(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    node_t *clone1 = node_clone(node->parent, node);
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
    return 1;
}


static int32_t
semantic_float32(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    node_t *clone1 = node_clone(node->parent, node);
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
    return 1;
}

static int32_t
semantic_float64(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    node_t *clone1 = node_clone(node->parent, node);
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
    return 1;
}

static int32_t
semantic_bigfloat(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    node_t *clone1 = node_clone(node->parent, node);
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
    return 1;
}


static int32_t
semantic_char(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    node_t *clone1 = node_clone(node->parent, node);
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
    return 1;
}

static int32_t
semantic_string(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    node_t *clone1 = node_clone(node->parent, node);
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
    return 1;
}

static int32_t
semantic_null(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    node_t *clone1 = node_clone(node->parent, node);
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
    return 1;
}


static int32_t
semantic_kint8(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    ilist_t *il1 = list_rpush(response, node);
    if (il1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }
    return 1;
}

static int32_t
semantic_kint16(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    ilist_t *il1 = list_rpush(response, node);
    if (il1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }
    return 1;
}

static int32_t
semantic_kint32(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    ilist_t *il1 = list_rpush(response, node);
    if (il1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }
    return 1;
}

static int32_t
semantic_kint64(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    ilist_t *il1 = list_rpush(response, node);
    if (il1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }
    return 1;
}


static int32_t
semantic_kuint8(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    ilist_t *il1 = list_rpush(response, node);
    if (il1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }
    return 1;
}

static int32_t
semantic_kuint16(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    ilist_t *il1 = list_rpush(response, node);
    if (il1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }
    return 1;
}

static int32_t
semantic_kuint32(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    ilist_t *il1 = list_rpush(response, node);
    if (il1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }
    return 1;
}

static int32_t
semantic_kuint64(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    ilist_t *il1 = list_rpush(response, node);
    if (il1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }
    return 1;
}

static int32_t
semantic_kbigint(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    ilist_t *il1 = list_rpush(response, node);
    if (il1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }
    return 1;
}


static int32_t
semantic_kfloat32(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    ilist_t *il1 = list_rpush(response, node);
    if (il1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }
    return 1;
}

static int32_t
semantic_kfloat64(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    ilist_t *il1 = list_rpush(response, node);
    if (il1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }
    return 1;
}

static int32_t
semantic_kbigfloat(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    ilist_t *il1 = list_rpush(response, node);
    if (il1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }
    return 1;
}


static int32_t
semantic_kchar(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    ilist_t *il1 = list_rpush(response, node);
    if (il1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }
    return 1;
}

static int32_t
semantic_kstring(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    ilist_t *il1 = list_rpush(response, node);
    if (il1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }
    return 1;
}



static int32_t
semantic_this(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    node_t *node1 = node;
    
    while (node1 != NULL)
    {
        if (node1->kind == NODE_KIND_CLASS)
        {
            break;
        }
        node1 = node1->parent;
    }

    if (node1 != NULL)
    {
        if ((node1->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
        {
            node_class_t *class1 = (node_class_t *)node1->value;

            node_t *key2 = class1->key;
            node_basic_t *key_string2 = key2->value;

            semantic_error(program, node, "Typing:'%s' does not have an instance type of '%s'\n\tInternal:%s-%u",
                "this", key_string2->value, __FILE__, __LINE__);
            return -1;
        }

        ilist_t *il1 = list_rpush(response, node1);
        if (il1 == NULL)
        {
            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }
        return 1;
    }

    return 0;
}

static int32_t
semantic_self(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    node_t *node1 = node;
    
    while (node1 != NULL)
    {
        if (node1->kind == NODE_KIND_LAMBDA)
        {
            break;
        }
        else
        if (node1->kind == NODE_KIND_FUN)
        {
            break;
        }
        node1 = node1->parent;
    }

    if (node1 != NULL)
    {
        if ((node1->flag & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
        {
            if (node1->kind == NODE_KIND_FUN)
            {
                node_fun_t *fun1 = (node_fun_t *)node1->value;

                node_t *key2 = fun1->key;
                node_basic_t *key_string2 = key2->value;

                semantic_error(program, node, "Typing:'%s' does not have an instance type of '%s'\n\tInternal:%s-%u",
                    "self", key_string2->value, __FILE__, __LINE__);
                return -1;
            }
            else
            {
                semantic_error(program, node, "Typing:'%s' does not have an instance type of '%s'\n\tInternal:%s-%u",
                    "self", "lambda", __FILE__, __LINE__);
                return -1;
            }
        }

        ilist_t *il1 = list_rpush(response, node1);
        if (il1 == NULL)
        {
            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }
        return 1;
    }

    return 0;
}

static int32_t
semantic_tuple(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    node_block_t *block1 = (node_block_t *)node->value;
    
    int32_t is_instance = 0;
    uint64_t cnt_item1 = 0;

    ilist_t *a1;
    for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;

        cnt_item1 += 1;

        list_t *response1 = list_create();
        if (response1 == NULL)
        {
            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = semantic_expression(program, item1, response1, flag);
        if (r1 == -1)
        {
            return -1;
        }
        else
        {
            uint64_t cnt_response1 = 0;

            ilist_t *a2;
            for (a2 = response1->begin;a2 != response1->end;a2 = a2->next)
            {
                cnt_response1 += 1;

                node_t *item2 = (node_t *)a2->value;
                
                if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                {
                    if (is_instance == 0)
                    {
                        if (cnt_item1 > 1)
                        {
                            semantic_error(program, item1, "Typing:Heterogeneous, all members of the tuple type must be non-instance\n\tInternal:%s-%u", 
                                __FILE__, __LINE__);
                            return -1;
                        }
                    }
                    is_instance = 1;
                }
                else
                {
                    if (is_instance == 1)
                    {
                        if (cnt_item1 > 1)
                        {
                            semantic_error(program, item1, "Typing:Heterogeneous, all members of the array must be instance\n\tInternal:%s-%u", 
                                __FILE__, __LINE__);
                            return -1;
                        }
                    }
                }
            }

            if (cnt_response1 == 0)
            {
                semantic_error(program, item1, "type of '%s' not found\n\tInternal:%s-%u", 
                    "item", __FILE__, __LINE__);
                return -1;
            }

            if (a1->next == block1->list->end)
            {
                if (is_instance == 1)
                {
                    node_t *node1 = node_create(node, node->position);
                    if (node1 == NULL)
                    {
                        return -1;
                    }

                    node1 = node_make_id(node1, "Array");
                    if (node1 == NULL)
                    {
                        return -1;
                    }

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

                        ilist_t *a3;
                        for (a3 = response2->begin;a3 != response2->end;a3 = a3->next)
                        {
                            cnt_response2 += 1;

                            node_t *item2 = (node_t *)a3->value;
                            if (item2->kind == NODE_KIND_CLASS)
                            {
                                if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    node_t *key1 = node1;
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
                                list_destroy(response1);
                                return 1;
                            }
                            else
                            {
                                semantic_error(program, node1, "Typing:'Array' has class type\n\tInternal:%s-%u",__FILE__, __LINE__);
                                return -1;
                            }
                        }

                        if (cnt_response2 == 0)
                        {
                            node_t *key1 = node1;
                            node_basic_t *key_string1 = key1->value;

                            semantic_error(program, node1, "Reference:'%s' not found\n\tInternal:%s-%u", 
                                key_string1->value, __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    list_destroy(response2);
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
            }
        }

        list_destroy(response1);
    }

    return 1;
}

static int32_t
semantic_object(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    node_block_t *block1 = (node_block_t *)node->value;
    
    int32_t is_set = 0, is_object = 0, is_instance = 0;
    uint64_t cnt_item1 = 0;

    ilist_t *a1;
    for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
    {
        node_t *item1 = (node_t *)a1->value;
        if (item1->kind == NODE_KIND_PAIR)
        {
            cnt_item1 += 1;

            node_pair_t *pair1 = (node_pair_t *)item1->value;
            if (pair1->value != NULL)
            {
                if (is_set == 1)
                {
                    semantic_error(program, item1, "Typing:Heterogeneous, all members of the set must be the same shape\n\tInternal:%s-%u", 
                        __FILE__, __LINE__);
                    return -1;
                }

                node_t *key1 = pair1->key;
                if (key1->kind == NODE_KIND_ID)
                {
                    if (cnt_item1 > 1)
                    {
                        if (is_object == 0)
                        {
                            semantic_error(program, item1, "Typing:Heterogeneous, all members of the dictionary must be the same shape\n\tInternal:%s-%u", 
                                __FILE__, __LINE__);
                            return -1;
                        }
                    }
                    is_object = 1;

                    list_t *response1 = list_create();
                    if (response1 == NULL)
                    {
                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r1 = semantic_expression(program, pair1->value, response1, flag);
                    if (r1 == -1)
                    {
                        return -1;
                    }
                    else
                    {
                        uint64_t cnt_response1 = 0;

                        ilist_t *a2;
                        for (a2 = response1->begin;a2 != response1->end;a2 = a2->next)
                        {
                            cnt_response1 += 1;

                            node_t *item2 = (node_t *)a2->value;
                            
                            if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                            {
                                if (is_instance == 0)
                                {
                                    if (cnt_item1 > 1)
                                    {
                                        semantic_error(program, key1, "Typing:Heterogeneous, all members of the object type must be non-instance\n\tInternal:%s-%u", 
                                            __FILE__, __LINE__);
                                        return -1;
                                    }
                                }
                                is_instance = 1;
                            }
                            else
                            {
                                if (is_instance == 1)
                                {
                                    if (cnt_item1 > 1)
                                    {
                                        semantic_error(program, key1, "Typing:Heterogeneous, all members of the object must be instance\n\tInternal:%s-%u", 
                                            __FILE__, __LINE__);
                                        return -1;
                                    }
                                }
                            }
                        }

                        if (cnt_response1 == 0)
                        {
                            node_basic_t *basic1 = (node_basic_t *)key1->value;
                            semantic_error(program, pair1->value, "type of '%s' not found\n\tInternal:%s-%u", 
                                basic1->value, __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    list_destroy(response1);

                    if (a1->next == block1->list->end)
                    {
                        if (is_instance == 1)
                        {
                            node_t *clone1 = node_clone(node->parent, node);
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
                            return 1;
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
                    }
                }
                else
                {
                    if (is_object == 1)
                    {
                        semantic_error(program, item1, "Typing:Heterogeneous, all members of the object must be the same shape\n\tInternal:%s-%u", 
                            __FILE__, __LINE__);
                        return -1;
                    }

                    if ((key1->kind == NODE_KIND_LAMBDA) || (key1->kind == NODE_KIND_OBJECT))
                    {
                        semantic_error(program, key1, "Typing:Invalid key, all keys of the dictionary must be of valid type\n\tInternal:%s-%u", 
                            __FILE__, __LINE__);
                        return -1;
                    }

                    if (a1->next == block1->list->end)
                    {
                        node_t *node1 = node_create(node, node->position);
                        if (node1 == NULL)
                        {
                            return -1;
                        }

                        node1 = node_make_id(node1, "Dictionary");
                        if (node1 == NULL)
                        {
                            return -1;
                        }

                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }

                        int32_t r1 = semantic_expression(program, node1, response1, flag);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        {
                            uint64_t cnt_response1 = 0;

                            ilist_t *a2;
                            for (a2 = response1->begin;a2 != response1->end;a2 = a2->next)
                            {
                                cnt_response1 += 1;

                                node_t *item2 = (node_t *)a2->value;
                                if (item2->kind == NODE_KIND_CLASS)
                                {
                                    if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                    {
                                        node_t *key1 = node1;
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
                                    list_destroy(response1);
                                    return 1;
                                }
                                else
                                {
                                    semantic_error(program, node1, "'Dictionary' has class type\n\tInternal:%s-%u",__FILE__, __LINE__);
                                    return -1;
                                }
                            }

                            if (cnt_response1 == 0)
                            {
                                node_t *key1 = node1;
                                node_basic_t *key_string1 = key1->value;

                                semantic_error(program, node1, "Reference:'%s' not found\n\tInternal:%s-%u", 
                                    key_string1->value, __FILE__, __LINE__);
                                return -1;
                            }
                        }

                        list_destroy(response1);
                    }
                }
            }
            else
            {
                if (is_set == 0)
                {
                    if (cnt_item1 > 1)
                    {
                        if (is_object == 1)
                        {
                            semantic_error(program, item1, "Typing:Heterogeneous, all members of the object must be the same shape\n\tInternal:%s-%u", 
                                __FILE__, __LINE__);
                            return -1;
                        }
                        else
                        {
                            semantic_error(program, item1, "Typing:Heterogeneous, all members of the dictionary must be the same shape\n\tInternal:%s-%u", 
                                __FILE__, __LINE__);
                            return -1;
                        }
                    }
                }
                is_set = 1;

                if (a1->next == block1->list->end)
                {
                    node_t *node1 = node_create(node, node->position);
                    if (node1 == NULL)
                    {
                        return -1;
                    }

                    node1 = node_make_id(node1, "Set");
                    if (node1 == NULL)
                    {
                        return -1;
                    }

                    list_t *response1 = list_create();
                    if (response1 == NULL)
                    {
                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }

                    int32_t r1 = semantic_expression(program, node1, response1, flag);
                    if (r1 == -1)
                    {
                        return -1;
                    }
                    else
                    {
                        uint64_t cnt_response1 = 0;

                        ilist_t *a2;
                        for (a2 = response1->begin;a2 != response1->end;a2 = a2->next)
                        {
                            cnt_response1 += 1;

                            node_t *item2 = (node_t *)a2->value;
                            if (item2->kind == NODE_KIND_CLASS)
                            {
                                if ((item2->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    node_t *key1 = node1;
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
                                list_destroy(response1);
                                return 1;
                            }
                            else
                            {
                                semantic_error(program, node1, "Typing:'Set' has class type\n\tInternal:%s-%u",__FILE__, __LINE__);
                                return -1;
                            }
                        }

                        if (cnt_response1 == 0)
                        {
                            node_t *key1 = node1;
                            node_basic_t *key_string1 = key1->value;

                            semantic_error(program, node1, "Reference:'%s' not found\n\tInternal:%s-%u", 
                                key_string1->value, __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    list_destroy(response1);
                }
            }
        }
    }
    
    ilist_t *il1 = list_rpush(response, node);
    if (il1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }
    return 1;
}

static int32_t
semantic_lambda(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    node_lambda_t *fun1 = (node_lambda_t *)node->value;
    if (fun1->body != NULL)
    {
        node_t *clone1 = node_clone(node->parent, node);
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
        return 1;
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
}

static int32_t
semantic_parenthesis(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    semantic_error(program, node, "Typing:it is wrong to use parenthesis in typing\n\tInternal:%s-%u", 
        __FILE__, __LINE__);
    return -1;
}

int32_t
semantic_primary(program_t *program, node_t *base, node_t *node, list_t *response, uint64_t flag)
{
    if (node->kind == NODE_KIND_ID)
    {
        return semantic_id(program, base, node, response, flag);
    }
    else

    if (node->kind == NODE_KIND_INT8)
    {
        return semantic_int8(program, node, response, flag);
    }
    else
    if (node->kind == NODE_KIND_INT16)
    {
        return semantic_int16(program, node, response, flag);
    }
    else
    if (node->kind == NODE_KIND_INT32)
    {
        return semantic_int32(program, node, response, flag);
    }
    else
    if (node->kind == NODE_KIND_INT64)
    {
        return semantic_int64(program, node, response, flag);
    }
    else

    if (node->kind == NODE_KIND_UINT8)
    {
        return semantic_uint8(program, node, response, flag);
    }
    else
    if (node->kind == NODE_KIND_UINT16)
    {
        return semantic_uint16(program, node, response, flag);
    }
    else
    if (node->kind == NODE_KIND_UINT32)
    {
        return semantic_uint32(program, node, response, flag);
    }
    else
    if (node->kind == NODE_KIND_UINT64)
    {
        return semantic_uint64(program, node, response, flag);
    }
    else
    if (node->kind == NODE_KIND_BIGINT)
    {
        return semantic_bigint(program, node, response, flag);
    }
    else

    if (node->kind == NODE_KIND_FLOAT32)
    {
        return semantic_float32(program, node, response, flag);
    }
    else
    if (node->kind == NODE_KIND_FLOAT64)
    {
        return semantic_float64(program, node, response, flag);
    }
    else
    if (node->kind == NODE_KIND_BIGFLOAT)
    {
        return semantic_bigfloat(program, node, response, flag);
    }
    else

    if (node->kind == NODE_KIND_CHAR)
    {
        return semantic_char(program, node, response, flag);
    }
    else
    if (node->kind == NODE_KIND_STRING)
    {
        return semantic_string(program, node, response, flag);
    }
    else

    if (node->kind == NODE_KIND_NULL)
    {
        return semantic_null(program, node, response, flag);
    }
    else

    if (node->kind == NODE_KIND_KINT8)
    {
        return semantic_kint8(program, node, response, flag);
    }
    else
    if (node->kind == NODE_KIND_KINT16)
    {
        return semantic_kint16(program, node, response, flag);
    }
    else
    if (node->kind == NODE_KIND_KINT32)
    {
        return semantic_kint32(program, node, response, flag);
    }
    else
    if (node->kind == NODE_KIND_KINT64)
    {
        return semantic_kint64(program, node, response, flag);
    }
    else

    if (node->kind == NODE_KIND_KUINT8)
    {
        return semantic_kuint8(program, node, response, flag);
    }
    else
    if (node->kind == NODE_KIND_KUINT16)
    {
        return semantic_kuint16(program, node, response, flag);
    }
    else
    if (node->kind == NODE_KIND_KUINT32)
    {
        return semantic_kuint32(program, node, response, flag);
    }
    else
    if (node->kind == NODE_KIND_KUINT64)
    {
        return semantic_kuint64(program, node, response, flag);
    }
    else
    if (node->kind == NODE_KIND_KBIGINT)
    {
        return semantic_kbigint(program, node, response, flag);
    }
    else

    if (node->kind == NODE_KIND_KFLOAT32)
    {
        return semantic_kfloat32(program, node, response, flag);
    }
    else
    if (node->kind == NODE_KIND_KFLOAT64)
    {
        return semantic_kfloat64(program, node, response, flag);
    }
    else
    if (node->kind == NODE_KIND_KBIGFLOAT)
    {
        return semantic_kbigfloat(program, node, response, flag);
    }
    else

    if (node->kind == NODE_KIND_KCHAR)
    {
        return semantic_kchar(program, node, response, flag);
    }
    else
    if (node->kind == NODE_KIND_KSTRING)
    {
        return semantic_kstring(program, node, response, flag);
    }
    else


    if (node->kind == NODE_KIND_THIS)
    {
        return semantic_this(program, node, response, flag);
    }
    else
    if (node->kind == NODE_KIND_SELF)
    {
        return semantic_self(program, node, response, flag);
    }
    else
    if (node->kind == NODE_KIND_TUPLE)
    {
        return semantic_tuple(program, node, response, flag);
    }
    else
    if (node->kind == NODE_KIND_OBJECT)
    {
        return semantic_object(program, node, response, flag);
    }
    else
    if (node->kind == NODE_KIND_LAMBDA)
    {
        return semantic_lambda(program, node, response, flag);
    }
    else
    if (node->kind == NODE_KIND_PARENTHESIS)
    {
        return semantic_parenthesis(program, node, response, flag);
    }
    else
    {
        semantic_error(program, node, "Unselectable\n\tInternal:%s-%u", __FILE__, __LINE__);
        return -1;
    }
}
