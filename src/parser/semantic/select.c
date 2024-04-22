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
#include "../syntax.h"
#include "../error.h"
#include "../semantic.h"

int32_t
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
    if (base->kind == NODE_KIND_FORIN)
    {
        node_forin_t *for1 = (node_forin_t *)base->value;

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
            if (item1->kind == NODE_KIND_FORIN)
            {
                node_forin_t *for1 = (node_forin_t *)item1->value;
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
                                    if ((item3->kind & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
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
                                    node_t *node3 = item3;
                                    while (node3 != NULL)
                                    {
                                        node_generic_t *generic1 = (node_generic_t *)node3->value;
                                        if (generic1->type != NULL)
                                        {
                                            list_t *response3 = list_create();
                                            if (response3 == NULL)
                                            {
                                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }

                                            int32_t r2 = semantic_resolve(program, generic1->type->parent, generic1->type, response3, SEMANTIC_FLAG_NONE);
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
                                                        if ((item4->kind & NODE_FLAG_INSTANCE) != NODE_FLAG_INSTANCE)
                                                        {
                                                            semantic_error(program, generic1->type, "Instance object, in confronting with (%s-%lld:%lld)",
                                                                item4->position.path, item4->position.line, item4->position.column);
                                                            return -1;
                                                        }

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
                                                        node3 = NULL;
                                                    }
                                                    else
                                                    if (item4->kind == NODE_KIND_GENERIC)
                                                    {
                                                        node3 = item4;
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
