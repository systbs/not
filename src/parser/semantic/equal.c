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
semantic_equivalence(program_t *program, node_t *n1, node_t *n2)
{
    if (n2->kind == NODE_KIND_KINT8)
    {
        if (n1->kind == NODE_KIND_CLASS)
        {
            node_class_t *class1 = (node_class_t *)n1->value;
            node_t *node1 = class1->block;
            node_block_t *block1 = (node_block_t *)node1->value;

            ilist_t *a1;
            for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_FUN)
                {
                    node_fun_t *fun1 = (node_fun_t *)item1->value;

                    if (semantic_idstrcmp(fun1->key, "Constructor") == 1)
                    {
                        int32_t r1 = semantic_eqaul_psns(program, fun1->parameters, n2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            return 1;
                        }
                    }
                }
            }
            return 0;
        }
        else
        if (n1->kind == NODE_KIND_KINT8)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    if (n2->kind == NODE_KIND_KINT16)
    {
        if (n1->kind == NODE_KIND_CLASS)
        {
            node_class_t *class1 = (node_class_t *)n1->value;
            node_t *node1 = class1->block;
            node_block_t *block1 = (node_block_t *)node1->value;

            ilist_t *a1;
            for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_FUN)
                {
                    node_fun_t *fun1 = (node_fun_t *)item1->value;

                    if (semantic_idstrcmp(fun1->key, "Constructor") == 1)
                    {
                        int32_t r1 = semantic_eqaul_psns(program, fun1->parameters, n2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            return 1;
                        }
                    }
                }
            }
            return 0;
        }
        else
        if (n1->kind == NODE_KIND_KINT16)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KINT8)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    if (n2->kind == NODE_KIND_KINT32)
    {
        if (n1->kind == NODE_KIND_CLASS)
        {
            node_class_t *class1 = (node_class_t *)n1->value;
            node_t *node1 = class1->block;
            node_block_t *block1 = (node_block_t *)node1->value;

            ilist_t *a1;
            for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_FUN)
                {
                    node_fun_t *fun1 = (node_fun_t *)item1->value;

                    if (semantic_idstrcmp(fun1->key, "Constructor") == 1)
                    {
                        int32_t r1 = semantic_eqaul_psns(program, fun1->parameters, n2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            return 1;
                        }
                    }
                }
            }
            return 0;
        }
        else
        if (n1->kind == NODE_KIND_KINT32)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KINT16)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KINT8)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    if (n2->kind == NODE_KIND_KINT64)
    {
        printf("Constructor2 %d\n\n", n1->kind);
        if (n1->kind == NODE_KIND_CLASS)
        {
            node_class_t *class1 = (node_class_t *)n1->value;
            node_t *node1 = class1->block;
            node_block_t *block1 = (node_block_t *)node1->value;

            ilist_t *a1;
            for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_FUN)
                {
                    node_fun_t *fun1 = (node_fun_t *)item1->value;

                    if (semantic_idstrcmp(fun1->key, "Constructor") == 1)
                    {
                        int32_t r1 = semantic_eqaul_psns(program, fun1->parameters, n2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            return 1;
                        }
                    }
                }
            }
            return 0;
        }
        else
        if (n1->kind == NODE_KIND_KINT64)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KINT32)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KINT16)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KINT8)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    if (n2->kind == NODE_KIND_KUINT8)
    {
        if (n1->kind == NODE_KIND_CLASS)
        {
            node_class_t *class1 = (node_class_t *)n1->value;
            node_t *node1 = class1->block;
            node_block_t *block1 = (node_block_t *)node1->value;

            ilist_t *a1;
            for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_FUN)
                {
                    node_fun_t *fun1 = (node_fun_t *)item1->value;

                    if (semantic_idstrcmp(fun1->key, "Constructor") == 1)
                    {
                        int32_t r1 = semantic_eqaul_psns(program, fun1->parameters, n2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            return 1;
                        }
                    }
                }
            }
            return 0;
        }
        else
        if (n1->kind == NODE_KIND_KUINT8)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    if (n2->kind == NODE_KIND_KUINT16)
    {
        if (n1->kind == NODE_KIND_CLASS)
        {
            node_class_t *class1 = (node_class_t *)n1->value;
            node_t *node1 = class1->block;
            node_block_t *block1 = (node_block_t *)node1->value;

            ilist_t *a1;
            for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_FUN)
                {
                    node_fun_t *fun1 = (node_fun_t *)item1->value;

                    if (semantic_idstrcmp(fun1->key, "Constructor") == 1)
                    {
                        int32_t r1 = semantic_eqaul_psns(program, fun1->parameters, n2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            return 1;
                        }
                    }
                }
            }
            return 0;
        }
        else
        if (n1->kind == NODE_KIND_KUINT16)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KUINT8)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    if (n2->kind == NODE_KIND_KUINT32)
    {
        if (n1->kind == NODE_KIND_CLASS)
        {
            node_class_t *class1 = (node_class_t *)n1->value;
            node_t *node1 = class1->block;
            node_block_t *block1 = (node_block_t *)node1->value;

            ilist_t *a1;
            for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_FUN)
                {
                    node_fun_t *fun1 = (node_fun_t *)item1->value;

                    if (semantic_idstrcmp(fun1->key, "Constructor") == 1)
                    {
                        int32_t r1 = semantic_eqaul_psns(program, fun1->parameters, n2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            return 1;
                        }
                    }
                }
            }
            return 0;
        }
        else
        if (n1->kind == NODE_KIND_KUINT32)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KUINT16)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KUINT8)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    if (n2->kind == NODE_KIND_KUINT64)
    {
        if (n1->kind == NODE_KIND_CLASS)
        {
            node_class_t *class1 = (node_class_t *)n1->value;
            node_t *node1 = class1->block;
            node_block_t *block1 = (node_block_t *)node1->value;

            ilist_t *a1;
            for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_FUN)
                {
                    node_fun_t *fun1 = (node_fun_t *)item1->value;

                    if (semantic_idstrcmp(fun1->key, "Constructor") == 1)
                    {
                        int32_t r1 = semantic_eqaul_psns(program, fun1->parameters, n2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            return 1;
                        }
                    }
                }
            }
            return 0;
        }
        else
        if (n1->kind == NODE_KIND_KUINT64)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KUINT32)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KUINT16)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KUINT8)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    if (n2->kind == NODE_KIND_KBIGINT)
    {
        if (n1->kind == NODE_KIND_CLASS)
        {
            node_class_t *class1 = (node_class_t *)n1->value;
            node_t *node1 = class1->block;
            node_block_t *block1 = (node_block_t *)node1->value;

            ilist_t *a1;
            for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_FUN)
                {
                    node_fun_t *fun1 = (node_fun_t *)item1->value;

                    if (semantic_idstrcmp(fun1->key, "Constructor") == 1)
                    {
                        int32_t r1 = semantic_eqaul_psns(program, fun1->parameters, n2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            return 1;
                        }
                    }
                }
            }
            return 0;
        }
        else
        if (n1->kind == NODE_KIND_KBIGINT)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KINT64)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KINT32)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KINT16)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KINT8)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KUINT64)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KUINT32)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KUINT16)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KUINT8)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    if (n2->kind == NODE_KIND_KFLOAT32)
    {
        if (n1->kind == NODE_KIND_CLASS)
        {
            node_class_t *class1 = (node_class_t *)n1->value;
            node_t *node1 = class1->block;
            node_block_t *block1 = (node_block_t *)node1->value;

            ilist_t *a1;
            for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_FUN)
                {
                    node_fun_t *fun1 = (node_fun_t *)item1->value;

                    if (semantic_idstrcmp(fun1->key, "Constructor") == 1)
                    {
                        int32_t r1 = semantic_eqaul_psns(program, fun1->parameters, n2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            return 1;
                        }
                    }
                }
            }
            return 0;
        }
        else
        if (n1->kind == NODE_KIND_KFLOAT32)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    if (n2->kind == NODE_KIND_KFLOAT64)
    {
        if (n1->kind == NODE_KIND_CLASS)
        {
            node_class_t *class1 = (node_class_t *)n1->value;
            node_t *node1 = class1->block;
            node_block_t *block1 = (node_block_t *)node1->value;

            ilist_t *a1;
            for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_FUN)
                {
                    node_fun_t *fun1 = (node_fun_t *)item1->value;

                    if (semantic_idstrcmp(fun1->key, "Constructor") == 1)
                    {
                        int32_t r1 = semantic_eqaul_psns(program, fun1->parameters, n2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            return 1;
                        }
                    }
                }
            }
            return 0;
        }
        else
        if (n1->kind == NODE_KIND_KFLOAT64)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KFLOAT32)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    if (n2->kind == NODE_KIND_KBIGFLOAT)
    {
        if (n1->kind == NODE_KIND_CLASS)
        {
            node_class_t *class1 = (node_class_t *)n1->value;
            node_t *node1 = class1->block;
            node_block_t *block1 = (node_block_t *)node1->value;

            ilist_t *a1;
            for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_FUN)
                {
                    node_fun_t *fun1 = (node_fun_t *)item1->value;

                    if (semantic_idstrcmp(fun1->key, "Constructor") == 1)
                    {
                        int32_t r1 = semantic_eqaul_psns(program, fun1->parameters, n2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            return 1;
                        }
                    }
                }
            }
            return 0;
        }
        else
        if (n1->kind == NODE_KIND_KBIGFLOAT)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KFLOAT64)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KFLOAT32)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    if (n2->kind == NODE_KIND_LAMBDA)
    {
        if (n1->kind == NODE_KIND_LAMBDA)
        {
            node_lambda_t *fun1 = (node_lambda_t *)n1->value;
            node_lambda_t *fun2 = (node_lambda_t *)n2->value;

            int32_t r1 = semantic_eqaul_gsgs(program, fun1->generics, fun2->generics);
            if (r1 == -1)
            {
                return -1;
            }
            else
            if (r1 == 1)
            {
                int32_t r2 = semantic_eqaul_psps(program, fun1->parameters, fun2->parameters);
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
        return 0;
    }
    else
    if (n2->kind == NODE_KIND_TUPLE)
    {
        if (n1->kind == NODE_KIND_CLASS)
        {
            node_class_t *class1 = (node_class_t *)n1->value;
            node_t *node1 = class1->block;
            node_block_t *block1 = (node_block_t *)node1->value;

            ilist_t *a1;
            for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_FUN)
                {
                    node_fun_t *fun1 = (node_fun_t *)item1->value;

                    if (semantic_idstrcmp(fun1->key, "Constructor") == 1)
                    {
                        int32_t r1 = semantic_eqaul_psns(program, fun1->parameters, n2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            return 1;
                        }
                    }
                }
            }
            return 0;
        }
        else
        if (n1->kind == NODE_KIND_TUPLE)
        {
            uint64_t cnt1 = 0, cnt2 = 0;
            
            node_block_t *block1 = (node_block_t *)n2->value;

            ilist_t *a1;
            for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;

                cnt1 += 1;

                node_block_t *block2 = (node_block_t *)n1->value;

                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    cnt2 += 1;
                    if (cnt2 < cnt1)
                    {
                        continue;
                    }

                    list_t *response1 = list_create();
                    if (response1 == NULL)
                    {
                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                    
                    int32_t r1 = semantic_expression(program, item1, response1, SEMANTIC_FLAG_NONE);
                    if (r1 == -1)
                    {
                        return -1;
                    }
                    else
                    if (r1 == 1)
                    {
                        uint64_t cnt_response1 = 0;

                        ilist_t *b1;
                        for (b1 = response1->begin;b1 != response1->end;b1 = b1->next)
                        {
                            cnt_response1 += 1;

                            node_t *item3 = (node_t *)b1->value;

                            if (item3->kind == NODE_KIND_KINT8)
                            {}
                            else
                            if (item3->kind == NODE_KIND_KINT16)
                            {}
                            else
                            if (item3->kind == NODE_KIND_KINT32)
                            {}
                            else
                            if (item3->kind == NODE_KIND_KINT64)
                            {}
                            else
                            if (item3->kind == NODE_KIND_KUINT8)
                            {}
                            else
                            if (item3->kind == NODE_KIND_KUINT16)
                            {}
                            else
                            if (item3->kind == NODE_KIND_KUINT32)
                            {}
                            else
                            if (item3->kind == NODE_KIND_KUINT64)
                            {}
                            else
                            if (item3->kind == NODE_KIND_KBIGINT)
                            {}
                            else
                            if (item3->kind == NODE_KIND_KFLOAT32)
                            {}
                            else
                            if (item3->kind == NODE_KIND_KFLOAT64)
                            {}
                            else
                            if (item3->kind == NODE_KIND_KBIGFLOAT)
                            {}
                            else
                            if (item3->kind == NODE_KIND_GENERIC)
                            {
                                if ((item3->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    node_generic_t *generic3 = (node_generic_t *)item3->value;

                                    node_t *key2 = generic3->key;
                                    node_basic_t *key_string2 = key2->value;

                                    semantic_error(program, item1, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                                        "tuple item", key_string2->value, __FILE__, __LINE__);
                                    return -1;
                                }
                            }
                            else
                            if (item3->kind == NODE_KIND_CLASS)
                            {
                                if ((item3->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    node_class_t *class1 = (node_class_t *)item3->value;

                                    node_t *key2 = class1->key;
                                    node_basic_t *key_string2 = key2->value;

                                    semantic_error(program, item1, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                                        "tuple item", key_string2->value, __FILE__, __LINE__);
                                    return -1;
                                }
                            }
                            else
                            if (item3->kind == NODE_KIND_TUPLE)
                            {
                                if ((item3->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    semantic_error(program, item1, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                                        "tuple item", "tuple", __FILE__, __LINE__);
                                    return -1;
                                }
                            }
                            else
                            if (item3->kind == NODE_KIND_OBJECT)
                            {
                                if ((item3->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    semantic_error(program, item1, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                                        "tuple item", "tuple", __FILE__, __LINE__);
                                    return -1;
                                }
                            }
                            else
                            if (item3->kind == NODE_KIND_ARRAY)
                            {
                                if ((item3->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    semantic_error(program, item1, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                                        "tuple item", "array", __FILE__, __LINE__);
                                    return -1;
                                }
                            }
                            else
                            {
                                semantic_error(program, item1, "Typing:'%s' has no valid type\n\tInternal:%s-%u",
                                    "item", __FILE__, __LINE__);
                                return -1;
                            }


                            list_t *response2 = list_create();
                            if (response2 == NULL)
                            {
                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }
                            
                            int32_t r2 = semantic_expression(program, item2, response2, SEMANTIC_FLAG_NONE);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                uint64_t cnt_response2 = 0;

                                ilist_t *b2;
                                for (b2 = response2->begin;b2 != response2->end;b2 = b2->next)
                                {
                                    cnt_response2 += 1;

                                    node_t *item4 = (node_t *)b2->value;

                                    if (item4->kind == NODE_KIND_KINT8)
                                    {}
                                    else
                                    if (item4->kind == NODE_KIND_KINT16)
                                    {}
                                    else
                                    if (item4->kind == NODE_KIND_KINT32)
                                    {}
                                    else
                                    if (item4->kind == NODE_KIND_KINT64)
                                    {}
                                    else
                                    if (item4->kind == NODE_KIND_KUINT8)
                                    {}
                                    else
                                    if (item4->kind == NODE_KIND_KUINT16)
                                    {}
                                    else
                                    if (item4->kind == NODE_KIND_KUINT32)
                                    {}
                                    else
                                    if (item4->kind == NODE_KIND_KUINT64)
                                    {}
                                    else
                                    if (item4->kind == NODE_KIND_KBIGINT)
                                    {}
                                    else
                                    if (item4->kind == NODE_KIND_KFLOAT32)
                                    {}
                                    else
                                    if (item4->kind == NODE_KIND_KFLOAT64)
                                    {}
                                    else
                                    if (item4->kind == NODE_KIND_KBIGFLOAT)
                                    {}
                                    else
                                    if (item4->kind == NODE_KIND_GENERIC)
                                    {
                                        if ((item4->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                        {
                                            node_generic_t *generic3 = (node_generic_t *)item4->value;

                                            node_t *key2 = generic3->key;
                                            node_basic_t *key_string2 = key2->value;

                                            semantic_error(program, item2, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                                                "tuple item", key_string2->value, __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }
                                    else
                                    if (item4->kind == NODE_KIND_CLASS)
                                    {
                                        if ((item4->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                        {
                                            node_class_t *class1 = (node_class_t *)item4->value;

                                            node_t *key2 = class1->key;
                                            node_basic_t *key_string2 = key2->value;

                                            semantic_error(program, item2, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                                                "tuple item", key_string2->value, __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }
                                    else
                                    if (item4->kind == NODE_KIND_TUPLE)
                                    {
                                        if ((item4->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                        {
                                            semantic_error(program, item2, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                                                "tuple item", "tuple", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }
                                    else
                                    if (item4->kind == NODE_KIND_OBJECT)
                                    {
                                        if ((item4->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                        {
                                            semantic_error(program, item2, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                                                "tuple item", "tuple", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }
                                    else
                                    if (item4->kind == NODE_KIND_ARRAY)
                                    {
                                        if ((item4->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                        {
                                            semantic_error(program, item2, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                                                "tuple item", "array", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }
                                    else
                                    {
                                        semantic_error(program, item2, "Typing:'%s' has no valid type\n\tInternal:%s-%u",
                                            "item", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r3 = semantic_equivalence(program, item3, item4);
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
                                }
                            
                                if (cnt_response2 == 0)
                                {
                                    semantic_error(program, item2, "Reference: type of '%s' not found\n\tInternal:%s-%u", 
                                        "tuple item", __FILE__, __LINE__);
                                    return -1;
                                }
                            }

                            list_destroy(response2);
                        }
                    
                        if (cnt_response1 == 0)
                        {
                            semantic_error(program, item1, "Reference: type of '%s' not found\n\tInternal:%s-%u", 
                                "tuple item", __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    list_destroy(response1);
                }

                if (cnt2 < cnt1)
                {
                    return 0;
                }
            }

            return 1;
        }
        return 0;
    }
    else
    if (n2->kind == NODE_KIND_OBJECT)
    {
        if (n1->kind == NODE_KIND_CLASS)
        {
            node_class_t *class1 = (node_class_t *)n1->value;
            node_t *node1 = class1->block;
            node_block_t *block1 = (node_block_t *)node1->value;

            ilist_t *a1;
            for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_FUN)
                {
                    node_fun_t *fun1 = (node_fun_t *)item1->value;

                    if (semantic_idstrcmp(fun1->key, "Constructor") == 1)
                    {
                        int32_t r1 = semantic_eqaul_psns(program, fun1->parameters, n2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            return 1;
                        }
                    }
                }
            }
            return 0;
        }
        else
        if (n1->kind == NODE_KIND_OBJECT)
        {
            node_block_t *block1 = (node_block_t *)n2->value;

            ilist_t *a1;
            for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_PAIR)
                {
                    node_pair_t *pair1 = (node_pair_t *)item1->value;
                    
                    int32_t found = 0;
                    node_block_t *block2 = (node_block_t *)n1->value;

                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_PAIR)
                        {
                            node_pair_t *pair2 = (node_pair_t *)item2->value;
                            
                            if (semantic_idcmp(pair1->key, pair2->key) == 1)
                            {
                                found = 1;

                                list_t *response1 = list_create();
                                if (response1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                
                                int32_t r1 = semantic_expression(program, pair1->value, response1, SEMANTIC_FLAG_NONE);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 1)
                                {
                                    uint64_t cnt_response1 = 0;

                                    ilist_t *b1;
                                    for (b1 = response1->begin;b1 != response1->end;b1 = b1->next)
                                    {
                                        cnt_response1 += 1;

                                        node_t *item3 = (node_t *)b1->value;

                                        if (item3->kind == NODE_KIND_KINT8)
                                        {}
                                        else
                                        if (item3->kind == NODE_KIND_KINT16)
                                        {}
                                        else
                                        if (item3->kind == NODE_KIND_KINT32)
                                        {}
                                        else
                                        if (item3->kind == NODE_KIND_KINT64)
                                        {}
                                        else
                                        if (item3->kind == NODE_KIND_KUINT8)
                                        {}
                                        else
                                        if (item3->kind == NODE_KIND_KUINT16)
                                        {}
                                        else
                                        if (item3->kind == NODE_KIND_KUINT32)
                                        {}
                                        else
                                        if (item3->kind == NODE_KIND_KUINT64)
                                        {}
                                        else
                                        if (item3->kind == NODE_KIND_KBIGINT)
                                        {}
                                        else
                                        if (item3->kind == NODE_KIND_KFLOAT32)
                                        {}
                                        else
                                        if (item3->kind == NODE_KIND_KFLOAT64)
                                        {}
                                        else
                                        if (item3->kind == NODE_KIND_KBIGFLOAT)
                                        {}
                                        else
                                        if (item3->kind == NODE_KIND_GENERIC)
                                        {
                                            if ((item3->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                            {
                                                node_t *key1 = pair1->key;
                                                node_basic_t *key_string1 = key1->value;

                                                node_generic_t *generic3 = (node_generic_t *)item3->value;

                                                node_t *key2 = generic3->key;
                                                node_basic_t *key_string2 = key2->value;

                                                semantic_error(program, pair1->key, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                                                    key_string1->value, key_string2->value, __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }
                                        else
                                        if (item3->kind == NODE_KIND_CLASS)
                                        {
                                            if ((item3->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                            {
                                                node_t *key1 = pair1->key;
                                                node_basic_t *key_string1 = key1->value;

                                                node_class_t *class1 = (node_class_t *)item3->value;

                                                node_t *key2 = class1->key;
                                                node_basic_t *key_string2 = key2->value;

                                                semantic_error(program, pair1->key, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                                                    key_string1->value, key_string2->value, __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }
                                        else
                                        if (item3->kind == NODE_KIND_TUPLE)
                                        {
                                            if ((item3->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                            {
                                                node_t *key1 = pair1->key;
                                                node_basic_t *key_string1 = key1->value;

                                                semantic_error(program, pair1->key, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                                                    key_string1->value, "tuple", __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }
                                        else
                                        if (item3->kind == NODE_KIND_OBJECT)
                                        {
                                            if ((item3->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                            {
                                                node_t *key1 = pair1->key;
                                                node_basic_t *key_string1 = key1->value;

                                                semantic_error(program, pair1->key, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                                                    key_string1->value, "tuple", __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }
                                        else
                                        if (item3->kind == NODE_KIND_ARRAY)
                                        {
                                            if ((item3->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                            {
                                                node_t *key1 = pair1->key;
                                                node_basic_t *key_string1 = key1->value;

                                                semantic_error(program, pair1->key, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                                                    key_string1->value, "array", __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }
                                        else
                                        {
                                            node_t *key1 = pair1->key;
                                            node_basic_t *key_string1 = key1->value;

                                            semantic_error(program, pair1->key, "Typing:'%s' has no valid type\n\tInternal:%s-%u",
                                                key_string1->value, __FILE__, __LINE__);
                                            return -1;
                                        }


                                        list_t *response2 = list_create();
                                        if (response2 == NULL)
                                        {
                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }
                                        
                                        int32_t r2 = semantic_expression(program, pair2->value, response2, SEMANTIC_FLAG_NONE);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            uint64_t cnt_response2 = 0;

                                            ilist_t *b2;
                                            for (b2 = response2->begin;b2 != response2->end;b2 = b2->next)
                                            {
                                                cnt_response2 += 1;

                                                node_t *item4 = (node_t *)b2->value;

                                                if (item4->kind == NODE_KIND_KINT8)
                                                {}
                                                else
                                                if (item4->kind == NODE_KIND_KINT16)
                                                {}
                                                else
                                                if (item4->kind == NODE_KIND_KINT32)
                                                {}
                                                else
                                                if (item4->kind == NODE_KIND_KINT64)
                                                {}
                                                else
                                                if (item4->kind == NODE_KIND_KUINT8)
                                                {}
                                                else
                                                if (item4->kind == NODE_KIND_KUINT16)
                                                {}
                                                else
                                                if (item4->kind == NODE_KIND_KUINT32)
                                                {}
                                                else
                                                if (item4->kind == NODE_KIND_KUINT64)
                                                {}
                                                else
                                                if (item4->kind == NODE_KIND_KBIGINT)
                                                {}
                                                else
                                                if (item4->kind == NODE_KIND_KFLOAT32)
                                                {}
                                                else
                                                if (item4->kind == NODE_KIND_KFLOAT64)
                                                {}
                                                else
                                                if (item4->kind == NODE_KIND_KBIGFLOAT)
                                                {}
                                                else
                                                if (item4->kind == NODE_KIND_GENERIC)
                                                {
                                                    if ((item4->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                    {
                                                        node_t *key1 = pair2->key;
                                                        node_basic_t *key_string1 = key1->value;

                                                        node_generic_t *generic3 = (node_generic_t *)item4->value;

                                                        node_t *key2 = generic3->key;
                                                        node_basic_t *key_string2 = key2->value;

                                                        semantic_error(program, pair2->key, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                                                            key_string1->value, key_string2->value, __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                }
                                                else
                                                if (item4->kind == NODE_KIND_CLASS)
                                                {
                                                    if ((item4->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                    {
                                                        node_t *key1 = pair2->key;
                                                        node_basic_t *key_string1 = key1->value;

                                                        node_class_t *class1 = (node_class_t *)item4->value;

                                                        node_t *key2 = class1->key;
                                                        node_basic_t *key_string2 = key2->value;

                                                        semantic_error(program, pair2->key, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                                                            key_string1->value, key_string2->value, __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                }
                                                else
                                                if (item4->kind == NODE_KIND_TUPLE)
                                                {
                                                    if ((item4->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                    {
                                                        node_t *key1 = pair2->key;
                                                        node_basic_t *key_string1 = key1->value;

                                                        semantic_error(program, pair2->key, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                                                            key_string1->value, "tuple", __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                }
                                                else
                                                if (item4->kind == NODE_KIND_OBJECT)
                                                {
                                                    if ((item4->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                    {
                                                        node_t *key1 = pair2->key;
                                                        node_basic_t *key_string1 = key1->value;

                                                        semantic_error(program, pair2->key, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                                                            key_string1->value, "tuple", __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                }
                                                else
                                                if (item4->kind == NODE_KIND_ARRAY)
                                                {
                                                    if ((item4->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                    {
                                                        node_t *key1 = pair2->key;
                                                        node_basic_t *key_string1 = key1->value;

                                                        semantic_error(program, pair2->key, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                                                            key_string1->value, "array", __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                }
                                                else
                                                {
                                                    node_t *key1 = pair2->key;
                                                    node_basic_t *key_string1 = key1->value;

                                                    semantic_error(program, pair2->key, "Typing:'%s' has no valid type\n\tInternal:%s-%u",
                                                        key_string1->value, __FILE__, __LINE__);
                                                    return -1;
                                                }

                                                int32_t r3 = semantic_equivalence(program, item3, item4);
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
                                            }
                                        
                                            if (cnt_response2 == 0)
                                            {
                                                node_t *key1 = pair2->key;
                                                node_basic_t *key_string1 = key1->value;

                                                semantic_error(program, pair2->key, "Reference: type of '%s' not found\n\tInternal:%s-%u", 
                                                    key_string1->value, __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }

                                        list_destroy(response2);
                                    }
                                
                                    if (cnt_response1 == 0)
                                    {
                                        node_t *key1 = pair1->key;
                                        node_basic_t *key_string1 = key1->value;

                                        semantic_error(program, pair1->key, "Reference: type of '%s' not found\n\tInternal:%s-%u", 
                                            key_string1->value, __FILE__, __LINE__);
                                        return -1;
                                    }
                                }

                                list_destroy(response1);
                            }
                        }
                    }

                    if (found == 0)
                    {
                        return 0;
                    }
                }
            }

            return 1;
        }

        return 0;
    }
    else
    if (n2->kind == NODE_KIND_ARRAY)
    {
        if (n1->kind == NODE_KIND_CLASS)
        {
            node_class_t *class1 = (node_class_t *)n1->value;
            node_t *node1 = class1->block;
            node_block_t *block1 = (node_block_t *)node1->value;

            ilist_t *a1;
            for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_FUN)
                {
                    node_fun_t *fun1 = (node_fun_t *)item1->value;

                    if (semantic_idstrcmp(fun1->key, "Constructor") == 1)
                    {
                        int32_t r1 = semantic_eqaul_psns(program, fun1->parameters, n2);
                        if (r1 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r1 == 1)
                        {
                            return 1;
                        }
                    }
                }
            }
            return 0;
        }
        else
        if (n1->kind == NODE_KIND_ARRAY)
        {
            node_carrier_t *carrier1 = (node_carrier_t *)n2->value;
            node_carrier_t *carrier2 = (node_carrier_t *)n1->value;

            int32_t r1 = semantic_equivalence(program, carrier2->base, carrier1->base);
            if (r1 == -1)
            {
                return -1;
            }
            else
            if (r1 == 0)
            {
                return 0;
            }

            return 1;
        }

        return 0;
    }
    else
    if (n2->kind == NODE_KIND_GENERIC)
    {
        node_generic_t *generic1 = (node_generic_t *)n2->value;
        if (generic1->type != NULL)
        {
            list_t *response1 = list_create();
            if (response1 == NULL)
            {
                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                return -1;
            }

            int32_t r1 = semantic_gresolve(program, n2, response1, SEMANTIC_FLAG_NONE);
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

                    node_t *item1 = (node_t *)a2->value;

                    int32_t r2 = semantic_equivalence(program, n1, item1);
                    if (r2 == -1)
                    {
                        return -1;
                    }
                    else
                    if (r2 == 1)
                    {
                        list_destroy(response1);
                        return 1;
                    }
                }

                if (cnt_response1 == 0)
                {
                    node_t *key1 = generic1->key;
                    node_basic_t *key_string1 = key1->value;

                    semantic_error(program, generic1->key, "Reference: type of '%s' not found\n\tInternal:%s-%u", 
                        key_string1->value, __FILE__, __LINE__);
                    return -1;
                }
            }
            list_destroy(response1);
        }
        else
        {
            return 1;
        }
    }
    else
    if (n2->kind == NODE_KIND_CLASS)
    {
        if (n1->kind == NODE_KIND_CLASS)
        {
            if (n1->id == n2->id)
            {
                return 1;
            }
        }
        else
        if (n1->kind == NODE_KIND_GENERIC)
        {
            node_generic_t *generic1 = (node_generic_t *)n1->value;
            if (generic1->type != NULL)
            {
                list_t *response1 = list_create();
                if (response1 == NULL)
                {
                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                    return -1;
                }

                int32_t r1 = semantic_gresolve(program, n1, response1, SEMANTIC_FLAG_NONE);
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

                        node_t *item1 = (node_t *)a2->value;

                        int32_t r2 = semantic_subset(program, item1, n2);
                        if (r2 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r2 == 1)
                        {
                            list_destroy(response1);
                            return 1;
                        }
                    }

                    if (cnt_response1 == 0)
                    {
                        node_t *key1 = generic1->key;
                        node_basic_t *key_string1 = key1->value;

                        semantic_error(program, generic1->key, "Reference: type of '%s' not found\n\tInternal:%s-%u", 
                            key_string1->value, __FILE__, __LINE__);
                        return -1;
                    }
                }
                list_destroy(response1);
            }
            else
            {
                return 1;
            }
        }

        node_class_t *class1 = (node_class_t *)n2->value;
        node_t *node1 = class1->block;
        node_block_t *block1 = (node_block_t *)node1->value;

        ilist_t *a1;
        for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
        {
            node_t *item1 = (node_t *)a1->value;
            if (item1->kind == NODE_KIND_FUN)
            {
                node_fun_t *fun1 = (node_fun_t *)item1->value;

                if (semantic_idstrcmp(fun1->key, "Constructor") == 1)
                {
                    int32_t r1 = semantic_eqaul_psns(program, fun1->parameters, n1);
                    if (r1 == -1)
                    {
                        return -1;
                    }
                    else
                    if (r1 == 1)
                    {
                        return 1;
                    }
                }
            }
        }

        return 0;
    }

    return 0;
}

int32_t
semantic_subset(program_t *program, node_t *n1, node_t *n2)
{
    if (n2->kind == NODE_KIND_KINT8)
    {
        if (n1->kind == NODE_KIND_KINT8)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    if (n2->kind == NODE_KIND_KINT16)
    {
        if (n1->kind == NODE_KIND_KINT16)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KINT8)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    if (n2->kind == NODE_KIND_KINT32)
    {
        if (n1->kind == NODE_KIND_KINT32)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KINT16)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KINT8)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    if (n2->kind == NODE_KIND_KINT64)
    {
        if (n1->kind == NODE_KIND_KINT64)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KINT32)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KINT16)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KINT8)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    if (n2->kind == NODE_KIND_KUINT8)
    {
        if (n1->kind == NODE_KIND_KUINT8)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    if (n2->kind == NODE_KIND_KUINT16)
    {
        if (n1->kind == NODE_KIND_KUINT16)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KUINT8)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    if (n2->kind == NODE_KIND_KUINT32)
    {
        if (n1->kind == NODE_KIND_KUINT32)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KUINT16)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KUINT8)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    if (n2->kind == NODE_KIND_KUINT64)
    {
        if (n1->kind == NODE_KIND_KUINT64)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KUINT32)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KUINT16)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KUINT8)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    if (n2->kind == NODE_KIND_KBIGINT)
    {
        if (n1->kind == NODE_KIND_KBIGINT)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KINT64)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KINT32)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KINT16)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KINT8)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KUINT64)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KUINT32)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KUINT16)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KUINT8)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    if (n2->kind == NODE_KIND_KFLOAT32)
    {
        if (n1->kind == NODE_KIND_KFLOAT32)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    if (n2->kind == NODE_KIND_KFLOAT64)
    {
        if (n1->kind == NODE_KIND_KFLOAT64)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KFLOAT32)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    if (n2->kind == NODE_KIND_KBIGFLOAT)
    {
        if (n1->kind == NODE_KIND_KBIGFLOAT)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KFLOAT64)
        {
            return 1;
        }
        else
        if (n1->kind == NODE_KIND_KFLOAT32)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    if (n2->kind == NODE_KIND_TUPLE)
    {
        if (n1->kind == NODE_KIND_TUPLE)
        {
            uint64_t cnt1 = 0, cnt2 = 0;
            
            node_block_t *block1 = (node_block_t *)n2->value;

            ilist_t *a1;
            for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;

                cnt1 += 1;

                node_block_t *block2 = (node_block_t *)n1->value;

                ilist_t *a2;
                for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                {
                    node_t *item2 = (node_t *)a2->value;
                    cnt2 += 1;
                    if (cnt2 < cnt1)
                    {
                        continue;
                    }

                    list_t *response1 = list_create();
                    if (response1 == NULL)
                    {
                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                        return -1;
                    }
                    
                    int32_t r1 = semantic_expression(program, item1, response1, SEMANTIC_FLAG_NONE);
                    if (r1 == -1)
                    {
                        return -1;
                    }
                    else
                    if (r1 == 1)
                    {
                        uint64_t cnt_response1 = 0;

                        ilist_t *b1;
                        for (b1 = response1->begin;b1 != response1->end;b1 = b1->next)
                        {
                            cnt_response1 += 1;

                            node_t *item3 = (node_t *)b1->value;

                            if (item3->kind == NODE_KIND_KINT8)
                            {}
                            else
                            if (item3->kind == NODE_KIND_KINT16)
                            {}
                            else
                            if (item3->kind == NODE_KIND_KINT32)
                            {}
                            else
                            if (item3->kind == NODE_KIND_KINT64)
                            {}
                            else
                            if (item3->kind == NODE_KIND_KUINT8)
                            {}
                            else
                            if (item3->kind == NODE_KIND_KUINT16)
                            {}
                            else
                            if (item3->kind == NODE_KIND_KUINT32)
                            {}
                            else
                            if (item3->kind == NODE_KIND_KUINT64)
                            {}
                            else
                            if (item3->kind == NODE_KIND_KBIGINT)
                            {}
                            else
                            if (item3->kind == NODE_KIND_KFLOAT32)
                            {}
                            else
                            if (item3->kind == NODE_KIND_KFLOAT64)
                            {}
                            else
                            if (item3->kind == NODE_KIND_KBIGFLOAT)
                            {}
                            else
                            if (item3->kind == NODE_KIND_GENERIC)
                            {
                                if ((item3->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    node_generic_t *generic3 = (node_generic_t *)item3->value;

                                    node_t *key2 = generic3->key;
                                    node_basic_t *key_string2 = key2->value;

                                    semantic_error(program, item1, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                                        "tuple item", key_string2->value, __FILE__, __LINE__);
                                    return -1;
                                }
                            }
                            else
                            if (item3->kind == NODE_KIND_CLASS)
                            {
                                if ((item3->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    node_class_t *class1 = (node_class_t *)item3->value;

                                    node_t *key2 = class1->key;
                                    node_basic_t *key_string2 = key2->value;

                                    semantic_error(program, item1, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                                        "tuple item", key_string2->value, __FILE__, __LINE__);
                                    return -1;
                                }
                            }
                            else
                            if (item3->kind == NODE_KIND_TUPLE)
                            {
                                if ((item3->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    semantic_error(program, item1, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                                        "tuple item", "tuple", __FILE__, __LINE__);
                                    return -1;
                                }
                            }
                            else
                            if (item3->kind == NODE_KIND_OBJECT)
                            {
                                if ((item3->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    semantic_error(program, item1, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                                        "tuple item", "tuple", __FILE__, __LINE__);
                                    return -1;
                                }
                            }
                            else
                            if (item3->kind == NODE_KIND_ARRAY)
                            {
                                if ((item3->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                {
                                    semantic_error(program, item1, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                                        "tuple item", "array", __FILE__, __LINE__);
                                    return -1;
                                }
                            }
                            else
                            {
                                semantic_error(program, item1, "Typing:'%s' has no valid type\n\tInternal:%s-%u",
                                    "item", __FILE__, __LINE__);
                                return -1;
                            }


                            list_t *response2 = list_create();
                            if (response2 == NULL)
                            {
                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }
                            
                            int32_t r2 = semantic_expression(program, item2, response2, SEMANTIC_FLAG_NONE);
                            if (r2 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r2 == 1)
                            {
                                uint64_t cnt_response2 = 0;

                                ilist_t *b2;
                                for (b2 = response2->begin;b2 != response2->end;b2 = b2->next)
                                {
                                    cnt_response2 += 1;

                                    node_t *item4 = (node_t *)b2->value;

                                    if (item4->kind == NODE_KIND_KINT8)
                                    {}
                                    else
                                    if (item4->kind == NODE_KIND_KINT16)
                                    {}
                                    else
                                    if (item4->kind == NODE_KIND_KINT32)
                                    {}
                                    else
                                    if (item4->kind == NODE_KIND_KINT64)
                                    {}
                                    else
                                    if (item4->kind == NODE_KIND_KUINT8)
                                    {}
                                    else
                                    if (item4->kind == NODE_KIND_KUINT16)
                                    {}
                                    else
                                    if (item4->kind == NODE_KIND_KUINT32)
                                    {}
                                    else
                                    if (item4->kind == NODE_KIND_KUINT64)
                                    {}
                                    else
                                    if (item4->kind == NODE_KIND_KBIGINT)
                                    {}
                                    else
                                    if (item4->kind == NODE_KIND_KFLOAT32)
                                    {}
                                    else
                                    if (item4->kind == NODE_KIND_KFLOAT64)
                                    {}
                                    else
                                    if (item4->kind == NODE_KIND_KBIGFLOAT)
                                    {}
                                    else
                                    if (item4->kind == NODE_KIND_GENERIC)
                                    {
                                        if ((item4->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                        {
                                            node_generic_t *generic3 = (node_generic_t *)item4->value;

                                            node_t *key2 = generic3->key;
                                            node_basic_t *key_string2 = key2->value;

                                            semantic_error(program, item2, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                                                "tuple item", key_string2->value, __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }
                                    else
                                    if (item4->kind == NODE_KIND_CLASS)
                                    {
                                        if ((item4->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                        {
                                            node_class_t *class1 = (node_class_t *)item4->value;

                                            node_t *key2 = class1->key;
                                            node_basic_t *key_string2 = key2->value;

                                            semantic_error(program, item2, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                                                "tuple item", key_string2->value, __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }
                                    else
                                    if (item4->kind == NODE_KIND_TUPLE)
                                    {
                                        if ((item4->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                        {
                                            semantic_error(program, item2, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                                                "tuple item", "tuple", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }
                                    else
                                    if (item4->kind == NODE_KIND_OBJECT)
                                    {
                                        if ((item4->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                        {
                                            semantic_error(program, item2, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                                                "tuple item", "tuple", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }
                                    else
                                    if (item4->kind == NODE_KIND_ARRAY)
                                    {
                                        if ((item4->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                        {
                                            semantic_error(program, item2, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                                                "tuple item", "array", __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }
                                    else
                                    {
                                        semantic_error(program, item2, "Typing:'%s' has no valid type\n\tInternal:%s-%u",
                                            "item", __FILE__, __LINE__);
                                        return -1;
                                    }

                                    int32_t r3 = semantic_subset(program, item4, item3);
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
                                }
                            
                                if (cnt_response2 == 0)
                                {
                                    semantic_error(program, item2, "Reference: type of '%s' not found\n\tInternal:%s-%u", 
                                        "tuple item", __FILE__, __LINE__);
                                    return -1;
                                }
                            }

                            list_destroy(response2);
                        }
                    
                        if (cnt_response1 == 0)
                        {
                            semantic_error(program, item1, "Reference: type of '%s' not found\n\tInternal:%s-%u", 
                                "tuple item", __FILE__, __LINE__);
                            return -1;
                        }
                    }

                    list_destroy(response1);
                }

                if (cnt2 < cnt1)
                {
                    return 0;
                }
            }

            return 1;
        }
        return 0;
    }
    else
    if (n2->kind == NODE_KIND_OBJECT)
    {
        if (n1->kind == NODE_KIND_OBJECT)
        {
            node_block_t *block1 = (node_block_t *)n2->value;

            ilist_t *a1;
            for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_PAIR)
                {
                    node_pair_t *pair1 = (node_pair_t *)item1->value;
                    
                    int32_t found = 0;
                    node_block_t *block2 = (node_block_t *)n1->value;

                    ilist_t *a2;
                    for (a2 = block2->list->begin;a2 != block2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_PAIR)
                        {
                            node_pair_t *pair2 = (node_pair_t *)item2->value;
                            
                            if (semantic_idcmp(pair1->key, pair2->key) == 1)
                            {
                                found = 1;

                                list_t *response1 = list_create();
                                if (response1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                
                                int32_t r1 = semantic_expression(program, pair1->value, response1, SEMANTIC_FLAG_NONE);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 1)
                                {
                                    uint64_t cnt_response1 = 0;

                                    ilist_t *b1;
                                    for (b1 = response1->begin;b1 != response1->end;b1 = b1->next)
                                    {
                                        cnt_response1 += 1;

                                        node_t *item3 = (node_t *)b1->value;

                                        if (item3->kind == NODE_KIND_KINT8)
                                        {}
                                        else
                                        if (item3->kind == NODE_KIND_KINT16)
                                        {}
                                        else
                                        if (item3->kind == NODE_KIND_KINT32)
                                        {}
                                        else
                                        if (item3->kind == NODE_KIND_KINT64)
                                        {}
                                        else
                                        if (item3->kind == NODE_KIND_KUINT8)
                                        {}
                                        else
                                        if (item3->kind == NODE_KIND_KUINT16)
                                        {}
                                        else
                                        if (item3->kind == NODE_KIND_KUINT32)
                                        {}
                                        else
                                        if (item3->kind == NODE_KIND_KUINT64)
                                        {}
                                        else
                                        if (item3->kind == NODE_KIND_KBIGINT)
                                        {}
                                        else
                                        if (item3->kind == NODE_KIND_KFLOAT32)
                                        {}
                                        else
                                        if (item3->kind == NODE_KIND_KFLOAT64)
                                        {}
                                        else
                                        if (item3->kind == NODE_KIND_KBIGFLOAT)
                                        {}
                                        else
                                        if (item3->kind == NODE_KIND_GENERIC)
                                        {
                                            if ((item3->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                            {
                                                node_t *key1 = pair1->key;
                                                node_basic_t *key_string1 = key1->value;

                                                node_generic_t *generic3 = (node_generic_t *)item3->value;

                                                node_t *key2 = generic3->key;
                                                node_basic_t *key_string2 = key2->value;

                                                semantic_error(program, pair1->key, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                                                    key_string1->value, key_string2->value, __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }
                                        else
                                        if (item3->kind == NODE_KIND_CLASS)
                                        {
                                            if ((item3->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                            {
                                                node_t *key1 = pair1->key;
                                                node_basic_t *key_string1 = key1->value;

                                                node_class_t *class1 = (node_class_t *)item3->value;

                                                node_t *key2 = class1->key;
                                                node_basic_t *key_string2 = key2->value;

                                                semantic_error(program, pair1->key, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                                                    key_string1->value, key_string2->value, __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }
                                        else
                                        if (item3->kind == NODE_KIND_TUPLE)
                                        {
                                            if ((item3->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                            {
                                                node_t *key1 = pair1->key;
                                                node_basic_t *key_string1 = key1->value;

                                                semantic_error(program, pair1->key, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                                                    key_string1->value, "tuple", __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }
                                        else
                                        if (item3->kind == NODE_KIND_OBJECT)
                                        {
                                            if ((item3->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                            {
                                                node_t *key1 = pair1->key;
                                                node_basic_t *key_string1 = key1->value;

                                                semantic_error(program, pair1->key, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                                                    key_string1->value, "tuple", __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }
                                        else
                                        if (item3->kind == NODE_KIND_ARRAY)
                                        {
                                            if ((item3->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                            {
                                                node_t *key1 = pair1->key;
                                                node_basic_t *key_string1 = key1->value;

                                                semantic_error(program, pair1->key, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                                                    key_string1->value, "array", __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }
                                        else
                                        {
                                            node_t *key1 = pair1->key;
                                            node_basic_t *key_string1 = key1->value;

                                            semantic_error(program, pair1->key, "Typing:'%s' has no valid type\n\tInternal:%s-%u",
                                                key_string1->value, __FILE__, __LINE__);
                                            return -1;
                                        }


                                        list_t *response2 = list_create();
                                        if (response2 == NULL)
                                        {
                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }
                                        
                                        int32_t r2 = semantic_expression(program, pair2->value, response2, SEMANTIC_FLAG_NONE);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            uint64_t cnt_response2 = 0;

                                            ilist_t *b2;
                                            for (b2 = response2->begin;b2 != response2->end;b2 = b2->next)
                                            {
                                                cnt_response2 += 1;

                                                node_t *item4 = (node_t *)b2->value;

                                                if (item4->kind == NODE_KIND_KINT8)
                                                {}
                                                else
                                                if (item4->kind == NODE_KIND_KINT16)
                                                {}
                                                else
                                                if (item4->kind == NODE_KIND_KINT32)
                                                {}
                                                else
                                                if (item4->kind == NODE_KIND_KINT64)
                                                {}
                                                else
                                                if (item4->kind == NODE_KIND_KUINT8)
                                                {}
                                                else
                                                if (item4->kind == NODE_KIND_KUINT16)
                                                {}
                                                else
                                                if (item4->kind == NODE_KIND_KUINT32)
                                                {}
                                                else
                                                if (item4->kind == NODE_KIND_KUINT64)
                                                {}
                                                else
                                                if (item4->kind == NODE_KIND_KBIGINT)
                                                {}
                                                else
                                                if (item4->kind == NODE_KIND_KFLOAT32)
                                                {}
                                                else
                                                if (item4->kind == NODE_KIND_KFLOAT64)
                                                {}
                                                else
                                                if (item4->kind == NODE_KIND_KBIGFLOAT)
                                                {}
                                                else
                                                if (item4->kind == NODE_KIND_GENERIC)
                                                {
                                                    if ((item4->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                    {
                                                        node_t *key1 = pair2->key;
                                                        node_basic_t *key_string1 = key1->value;

                                                        node_generic_t *generic3 = (node_generic_t *)item4->value;

                                                        node_t *key2 = generic3->key;
                                                        node_basic_t *key_string2 = key2->value;

                                                        semantic_error(program, pair2->key, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                                                            key_string1->value, key_string2->value, __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                }
                                                else
                                                if (item4->kind == NODE_KIND_CLASS)
                                                {
                                                    if ((item4->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                    {
                                                        node_t *key1 = pair2->key;
                                                        node_basic_t *key_string1 = key1->value;

                                                        node_class_t *class1 = (node_class_t *)item4->value;

                                                        node_t *key2 = class1->key;
                                                        node_basic_t *key_string2 = key2->value;

                                                        semantic_error(program, pair2->key, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                                                            key_string1->value, key_string2->value, __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                }
                                                else
                                                if (item4->kind == NODE_KIND_TUPLE)
                                                {
                                                    if ((item4->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                    {
                                                        node_t *key1 = pair2->key;
                                                        node_basic_t *key_string1 = key1->value;

                                                        semantic_error(program, pair2->key, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                                                            key_string1->value, "tuple", __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                }
                                                else
                                                if (item4->kind == NODE_KIND_OBJECT)
                                                {
                                                    if ((item4->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                    {
                                                        node_t *key1 = pair2->key;
                                                        node_basic_t *key_string1 = key1->value;

                                                        semantic_error(program, pair2->key, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                                                            key_string1->value, "tuple", __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                }
                                                else
                                                if (item4->kind == NODE_KIND_ARRAY)
                                                {
                                                    if ((item4->flag & NODE_FLAG_INSTANCE) == NODE_FLAG_INSTANCE)
                                                    {
                                                        node_t *key1 = pair2->key;
                                                        node_basic_t *key_string1 = key1->value;

                                                        semantic_error(program, pair2->key, "Typing:'%s' has an instance type of '%s'\n\tInternal:%s-%u",
                                                            key_string1->value, "array", __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                }
                                                else
                                                {
                                                    node_t *key1 = pair2->key;
                                                    node_basic_t *key_string1 = key1->value;

                                                    semantic_error(program, pair2->key, "Typing:'%s' has no valid type\n\tInternal:%s-%u",
                                                        key_string1->value, __FILE__, __LINE__);
                                                    return -1;
                                                }

                                                int32_t r3 = semantic_subset(program, item4, item3);
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
                                            }
                                        
                                            if (cnt_response2 == 0)
                                            {
                                                node_t *key1 = pair2->key;
                                                node_basic_t *key_string1 = key1->value;

                                                semantic_error(program, pair2->key, "Reference: type of '%s' not found\n\tInternal:%s-%u", 
                                                    key_string1->value, __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }

                                        list_destroy(response2);
                                    }
                                
                                    if (cnt_response1 == 0)
                                    {
                                        node_t *key1 = pair1->key;
                                        node_basic_t *key_string1 = key1->value;

                                        semantic_error(program, pair1->key, "Reference: type of '%s' not found\n\tInternal:%s-%u", 
                                            key_string1->value, __FILE__, __LINE__);
                                        return -1;
                                    }
                                }

                                list_destroy(response1);
                            }
                        }
                    }

                    if (found == 0)
                    {
                        return 0;
                    }
                }
            }

            return 1;
        }

        return 0;
    }
    else
    if (n2->kind == NODE_KIND_ARRAY)
    {
        if (n1->kind == NODE_KIND_ARRAY)
        {
            node_carrier_t *carrier1 = (node_carrier_t *)n2->value;
            node_carrier_t *carrier2 = (node_carrier_t *)n1->value;

            int32_t r1 = semantic_subset(program, carrier2->base, carrier1->base);
            if (r1 == -1)
            {
                return -1;
            }
            else
            if (r1 == 0)
            {
                return 0;
            }

            return 1;
        }

        return 0;
    }
    else
    if (n2->kind == NODE_KIND_GENERIC)
    {
        node_generic_t *generic1 = (node_generic_t *)n2->value;
        if (generic1->type != NULL)
        {
            list_t *response1 = list_create();
            if (response1 == NULL)
            {
                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                return -1;
            }

            int32_t r1 = semantic_gresolve(program, n2, response1, SEMANTIC_FLAG_NONE);
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

                    node_t *item1 = (node_t *)a2->value;

                    int32_t r2 = semantic_subset(program, n1, item1);
                    if (r2 == -1)
                    {
                        return -1;
                    }
                    else
                    if (r2 == 1)
                    {
                        list_destroy(response1);
                        return 1;
                    }
                }

                if (cnt_response1 == 0)
                {
                    node_t *key1 = generic1->key;
                    node_basic_t *key_string1 = key1->value;

                    semantic_error(program, generic1->key, "Reference: type of '%s' not found\n\tInternal:%s-%u", 
                        key_string1->value, __FILE__, __LINE__);
                    return -1;
                }
            }
            list_destroy(response1);
        }
        else
        {
            return 1;
        }
    }
    else
    if (n2->kind == NODE_KIND_CLASS)
    {
        int32_t r1 = semantic_equivalence(program, n1, n2);
        if (r1 == -1)
        {
            return -1;
        }
        if (r1 == 1)
        {
            return 1;
        }

        if (n1->kind == NODE_KIND_CLASS)
        {
            node_class_t *class1 = (node_class_t *)n2->value;
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

                        list_t *response1 = list_create();
                        if (response1 == NULL)
                        {
                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                            return -1;
                        }

                        int32_t r2 = semantic_hresolve(program, item1, response1, SEMANTIC_FLAG_NONE);
                        if (r2 == -1)
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

                                int32_t r3 = semantic_subset(program, n1, item2);
                                if (r3 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r3 == 1)
                                {
                                    list_destroy(response1);
                                    return 1;
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
                }
            }
        }
        else
        if (n1->kind == NODE_KIND_GENERIC)
        {
            node_generic_t *generic1 = (node_generic_t *)n1->value;
            if (generic1->type != NULL)
            {
                list_t *response1 = list_create();
                if (response1 == NULL)
                {
                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                    return -1;
                }

                int32_t r1 = semantic_gresolve(program, n1, response1, SEMANTIC_FLAG_NONE);
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

                        node_t *item1 = (node_t *)a2->value;

                        int32_t r2 = semantic_subset(program, item1, n2);
                        if (r2 == -1)
                        {
                            return -1;
                        }
                        else
                        if (r2 == 1)
                        {
                            list_destroy(response1);
                            return 1;
                        }
                    }

                    if (cnt_response1 == 0)
                    {
                        node_t *key1 = generic1->key;
                        node_basic_t *key_string1 = key1->value;

                        semantic_error(program, generic1->key, "Reference: type of '%s' not found\n\tInternal:%s-%u", 
                            key_string1->value, __FILE__, __LINE__);
                        return -1;
                    }
                }
                list_destroy(response1);
            }
            else
            {
                return 1;
            }
        }
    }

    return 0;
}

int32_t
semantic_eqaul_gsgs(program_t *program, node_t *ngs1, node_t *ngs2)
{
   if (ngs1 == NULL)
    {
        if (ngs2 == NULL)
        {
            return 1;
        }
        else
        {
            node_block_t *bgs2 = (node_block_t *)ngs2->value;
            ilist_t *a2;
            for (a2 = bgs2->list->begin;a2 != bgs2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->kind == NODE_KIND_GENERIC)
                {
                    node_generic_t *generic2 = (node_generic_t *)item2->value;
                    if (generic2->value == NULL)
                    {
                        return 0;
                    }
                }
            }
            return 1;
        }
    }
    else
    {
        if (ngs2 == NULL)
        {
            node_block_t *bps1 = (node_block_t *)ngs1->value;
            ilist_t *a1;
            for (a1 = bps1->list->begin;a1 != bps1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_GENERIC)
                {
                    node_generic_t *generic1 = (node_generic_t *)item1->value;
                    if (generic1->value == NULL)
                    {
                        return 0;
                    }
                }
            }
            return 1;
        }
        else
        {
            uint64_t cnt1 = 0, cnt2 = 0;
            cnt1 = 0;

            node_block_t *bps1 = (node_block_t *)ngs1->value;
            ilist_t *a1;
            for (a1 = bps1->list->begin;a1 != bps1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_GENERIC)
                {
                    cnt1 += 1;
                    cnt2 = 0;
                    node_generic_t *generic1 = (node_generic_t *)item1->value;
                    node_block_t *bgs2 = (node_block_t *)ngs2->value;
                    ilist_t *a2;
                    for (a2 = bgs2->list->begin;a2 != bgs2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_GENERIC)
                        {
                            cnt2 += 1;
                            if (cnt2 < cnt1)
                            {
                                continue;
                            }
                            node_generic_t *generic2 = (node_generic_t *)item2->value;
                            
                            if (generic1->type != NULL)
                            {
                                if (generic2->type != NULL)
                                {
                                    list_t *response1 = list_create();
                                    if (response1 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    
                                    int32_t r1 = semantic_expression(program, generic1->type, response1, SEMANTIC_FLAG_NONE);
                                    if (r1 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r1 == 1)
                                    {
                                        uint64_t cnt_response1 = 0;

                                        ilist_t *b1;
                                        for (b1 = response1->begin;b1 != response1->end;b1 = b1->next)
                                        {
                                            cnt_response1 += 1;

                                            node_t *item3 = (node_t *)b1->value;

                                            list_t *response2 = list_create();
                                            if (response2 == NULL)
                                            {
                                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                return -1;
                                            }
                                            
                                            int32_t r2 = semantic_expression(program, generic2->type, response2, SEMANTIC_FLAG_NONE);
                                            if (r2 == -1)
                                            {
                                                return -1;
                                            }
                                            else
                                            if (r2 == 1)
                                            {
                                                uint64_t cnt_response2 = 0;

                                                ilist_t *b2;
                                                for (b2 = response2->begin;b2 != response2->end;b2 = b2->next)
                                                {
                                                    cnt_response2 += 1;

                                                    node_t *item4 = (node_t *)b2->value;

                                                    int32_t r3 = semantic_subset(program, item3, item4);
                                                    if (r3 == -1)
                                                    {
                                                        return -1;
                                                    }
                                                    else
                                                    if (r3 == 0)
                                                    {
                                                        r3 = semantic_subset(program, item4, item3);
                                                        if (r3 == -1)
                                                        {
                                                            return -1;
                                                        }
                                                        else
                                                        if (r3 == 0)
                                                        {
                                                            list_destroy(response2);
                                                            list_destroy(response1);
                                                            goto region_by_name_check;
                                                        }
                                                        
                                                    }

                                                    if (cnt_response2 == 0)
                                                    {
                                                        node_t *key1 = generic2->key;
                                                        node_basic_t *key_string1 = key1->value;

                                                        semantic_error(program, generic2->key, "Reference: type of '%s' not found\n\tInternal:%s-%u", 
                                                            key_string1->value, __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                }
                                            }
                                            list_destroy(response2);
                                        }

                                        if (cnt_response1 == 0)
                                        {
                                            node_t *key1 = generic1->key;
                                            node_basic_t *key_string1 = key1->value;

                                            semantic_error(program, generic1->key, "Reference: type of '%s' not found\n\tInternal:%s-%u", 
                                                key_string1->value, __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }
                                    list_destroy(response1);
                                    break;
                               }
                            }
                        }
                    }

                    if (cnt1 > cnt2)
                    {
                        if (generic1->value == NULL)
                        {
                            goto region_by_name_check;
                        }
                    }
                }
            }

            cnt2 = 0;
            node_block_t *bgs2 = (node_block_t *)ngs2->value;
            ilist_t *a2;
            for (a2 = bgs2->list->begin;a2 != bgs2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->kind == NODE_KIND_GENERIC)
                {
                    cnt2 += 1;
                    if (cnt2 > cnt1)
                    {
                        node_generic_t *generic2 = (node_generic_t *)item2->value;
                        if (generic2->value == NULL)
                        {
                            goto region_by_name_check;
                        }
                    }
                }
            }

            return 1;

            region_by_name_check:
            for (a1 = bps1->list->begin;a1 != bps1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_GENERIC)
                {
                    node_generic_t *generic1 = (node_generic_t *)item1->value;

                    int32_t found = 0;
                    node_block_t *bgs2 = (node_block_t *)ngs2->value;
                    ilist_t *a2;
                    for (a2 = bgs2->list->begin;a2 != bgs2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_GENERIC)
                        {
                            node_generic_t *generic2 = (node_generic_t *)item2->value;
                            if (semantic_idcmp(generic1->key, generic2->key) == 1)
                            {
                                found = 1;
                                if (generic1->type != NULL)
                                {
                                    if (generic2->type != NULL)
                                    {
                                        list_t *response1 = list_create();
                                        if (response1 == NULL)
                                        {
                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }

                                        int32_t r1 = semantic_expression(program, generic1->type, response1, SEMANTIC_FLAG_NONE);
                                        if (r1 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r1 == 1)
                                        {
                                            uint64_t cnt_response1 = 0;

                                            ilist_t *b1;
                                            for (b1 = response1->begin;b1 != response1->end;b1 = b1->next)
                                            {
                                                cnt_response1 += 1;

                                                node_t *item3 = (node_t *)b1->value;

                                                list_t *response2 = list_create();
                                                if (response2 == NULL)
                                                {
                                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                
                                                int32_t r2 = semantic_expression(program, generic2->type, response2, SEMANTIC_FLAG_NONE);
                                                if (r2 == -1)
                                                {
                                                    return -1;
                                                }
                                                else
                                                if (r2 == 1)
                                                {
                                                    uint64_t cnt_response2 = 0;

                                                    ilist_t *b2;
                                                    for (b2 = response2->begin;b2 != response2->end;b2 = b2->next)
                                                    {
                                                        cnt_response2 += 1;

                                                        node_t *item4 = (node_t *)b2->value;
                                                        
                                                        int32_t r3 = semantic_subset(program, item3, item4);
                                                        if (r3 == -1)
                                                        {
                                                            return -1;
                                                        }
                                                        else
                                                        if (r3 == 0)
                                                        {
                                                            r3 = semantic_subset(program, item4, item3);
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
                                                        }
                                                    }

                                                    if (cnt_response2 == 0)
                                                    {
                                                        node_t *key1 = generic2->key;
                                                        node_basic_t *key_string1 = key1->value;

                                                        semantic_error(program, generic2->key, "Reference: type of '%s' not found\n\tInternal:%s-%u", 
                                                            key_string1->value, __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                }

                                                list_destroy(response2);
                                                break;
                                            }

                                            if (cnt_response1 == 0)
                                            {
                                                node_t *key1 = generic1->key;
                                                node_basic_t *key_string1 = key1->value;

                                                semantic_error(program, generic1->key, "Reference: type of '%s' not found\n\tInternal:%s-%u", 
                                                    key_string1->value, __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }
                                        list_destroy(response1);
                                    }
                                }
                            }
                        }
                    }

                    if (found == 0)
                    {
                        if (generic1->value == NULL)
                        {
                            return 0;
                        }
                    }
                }
            }

            bgs2 = (node_block_t *)ngs2->value;
            for (a2 = bgs2->list->begin;a2 != bgs2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->kind == NODE_KIND_GENERIC)
                {
                    node_generic_t *generic2 = (node_generic_t *)item2->value;

                    int32_t found = 0;
                    for (a1 = bps1->list->begin;a1 != bps1->list->end;a1 = a1->next)
                    {
                        node_t *item1 = (node_t *)a1->value;
                        if (item1->kind == NODE_KIND_GENERIC)
                        {
                            node_generic_t *generic1 = (node_generic_t *)item1->value;

                            if (semantic_idcmp(generic1->key, generic2->key) == 1)
                            {
                                found = 1;
                                if (generic1->type != NULL)
                                {
                                    if (generic2->type != NULL)
                                    {
                                        list_t *response1 = list_create();
                                        if (response1 == NULL)
                                        {
                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }

                                        int32_t r1 = semantic_expression(program, generic1->type, response1, SEMANTIC_FLAG_NONE);
                                        if (r1 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r1 == 1)
                                        {
                                            uint64_t cnt_response1 = 0;

                                            ilist_t *b1;
                                            for (b1 = response1->begin;b1 != response1->end;b1 = b1->next)
                                            {
                                                cnt_response1 += 1;

                                                node_t *item3 = (node_t *)b1->value;

                                                list_t *response2 = list_create();
                                                if (response2 == NULL)
                                                {
                                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                                    return -1;
                                                }
                                                
                                                int32_t r2 = semantic_expression(program, generic2->type, response2, SEMANTIC_FLAG_NONE);
                                                if (r2 == -1)
                                                {
                                                    return -1;
                                                }
                                                else
                                                if (r2 == 1)
                                                {
                                                    uint64_t cnt_response2 = 0;

                                                    ilist_t *b2;
                                                    for (b2 = response2->begin;b2 != response2->end;b2 = b2->next)
                                                    {
                                                        cnt_response2 += 1;

                                                        node_t *item4 = (node_t *)b2->value;
                                                        
                                                        int32_t r3 = semantic_subset(program, item3, item4);
                                                        if (r3 == -1)
                                                        {
                                                            return -1;
                                                        }
                                                        else
                                                        if (r3 == 0)
                                                        {
                                                            r3 = semantic_subset(program, item4, item3);
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
                                                        }
                                                    }

                                                    if (cnt_response2 == 0)
                                                    {
                                                        node_t *key1 = generic2->key;
                                                        node_basic_t *key_string1 = key1->value;

                                                        semantic_error(program, generic2->key, "Reference: type of '%s' not found\n\tInternal:%s-%u", 
                                                            key_string1->value, __FILE__, __LINE__);
                                                        return -1;
                                                    }
                                                }
                                                list_destroy(response2);
                                                break;
                                            }

                                            if (cnt_response1 == 0)
                                            {
                                                node_t *key1 = generic1->key;
                                                node_basic_t *key_string1 = key1->value;

                                                semantic_error(program, generic1->key, "Reference: type of '%s' not found\n\tInternal:%s-%u", 
                                                    key_string1->value, __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }
                                        list_destroy(response1);
                                    }
                                }
                            }
                        }
                    }

                    if (found == 0)
                    {
                        if (generic2->value == NULL)
                        {
                            return 0;
                        }
                    }
                }
            }

            return 1;
        }
    }
    return 0;
}

int32_t
semantic_eqaul_psps(program_t *program, node_t *nps1, node_t *nps2)
{
    if (nps1 == NULL)
    {
        if (nps2 == NULL)
        {
            return 1;
        }
        else
        {
            node_block_t *bps2 = (node_block_t *)nps2->value;
            ilist_t *a2;
            for (a2 = bps2->list->begin;a2 != bps2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->kind == NODE_KIND_PARAMETER)
                {
                    node_parameter_t *parameter2 = (node_parameter_t *)item2->value;
                    if (parameter2->value == NULL)
                    {
                        return 0;
                    }
                }
            }
            return 1;
        }
    }
    else
    {
        if (nps2 == NULL)
        {
            node_block_t *bps1 = (node_block_t *)nps1->value;
            ilist_t *a1;
            for (a1 = bps1->list->begin;a1 != bps1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_PARAMETER)
                {
                    node_parameter_t *parameter1 = (node_parameter_t *)item1->value;
                    if (parameter1->value == NULL)
                    {
                        return 0;
                    }
                }
            }
            return 1;
        }
        else
        {
            uint64_t cnt1, cnt2;
            cnt1 = 0;

            node_block_t *bps1 = (node_block_t *)nps1->value;
            ilist_t *a1;
            for (a1 = bps1->list->begin;a1 != bps1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_PARAMETER)
                {
                    cnt1 += 1;
                    cnt2 = 0;
                    node_parameter_t *parameter1 = (node_parameter_t *)item1->value;
                    node_block_t *bps2 = (node_block_t *)nps2->value;
                    ilist_t *a2;
                    for (a2 = bps2->list->begin;a2 != bps2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_PARAMETER)
                        {
                            cnt2 += 1;
                            if (cnt2 < cnt1)
                            {
                                continue;
                            }
                            
                            node_parameter_t *parameter2 = (node_parameter_t *)item2->value;
                            list_t *response1 = list_create();
                            if (response1 == NULL)
                            {
                                fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                return -1;
                            }
                            
                            int32_t r1 = semantic_expression(program, parameter1->type, response1, SEMANTIC_FLAG_NONE);
                            if (r1 == -1)
                            {
                                return -1;
                            }
                            else
                            if (r1 == 1)
                            {
                                uint64_t cnt_response1 = 0;

                                ilist_t *b1;
                                for (b1 = response1->begin;b1 != response1->end;b1 = b1->next)
                                {
                                    cnt_response1 += 1;

                                    node_t *item3 = (node_t *)b1->value;

                                    list_t *response2 = list_create();
                                    if (response2 == NULL)
                                    {
                                        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                        return -1;
                                    }
                                    
                                    int32_t r2 = semantic_expression(program, parameter2->type, response2, SEMANTIC_FLAG_NONE);
                                    if (r2 == -1)
                                    {
                                        return -1;
                                    }
                                    else
                                    if (r2 == 1)
                                    {
                                        uint64_t cnt_response2 = 0;

                                        ilist_t *b2;
                                        for (b2 = response2->begin;b2 != response2->end;b2 = b2->next)
                                        {
                                            cnt_response2 += 1;

                                            node_t *item4 = (node_t *)b2->value;
                                            
                                            int32_t r3 = semantic_equivalence(program, item3, item4);
                                            if (r3 == -1)
                                            {
                                                return -1;
                                            }
                                            else
                                            if (r3 == 0)
                                            {
                                                list_destroy(response2);
                                                list_destroy(response1);
                                                goto region_by_name_check;
                                            }
                                        }

                                        if (cnt_response2 == 0)
                                        {
                                            node_t *key1 = parameter2->key;
                                            node_basic_t *key_string1 = key1->value;

                                            semantic_error(program, parameter2->key, "Reference: type of '%s' not found\n\tInternal:%s-%u", 
                                                key_string1->value, __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }
                                     list_destroy(response2);
                                }

                                if (cnt_response1 == 0)
                                {
                                    node_t *key1 = parameter1->key;
                                    node_basic_t *key_string1 = key1->value;

                                    semantic_error(program, parameter1->key, "Reference: type of '%s' not found\n\tInternal:%s-%u", 
                                        key_string1->value, __FILE__, __LINE__);
                                    return -1;
                                }
                            }
                            list_destroy(response1);
                            break;
                        }
                    }

                    if (cnt1 > cnt2)
                    {
                        if (parameter1->value == NULL)
                        {
                            goto region_by_name_check;
                        }
                    }
                }
            }

            
            cnt2 = 0;
            node_block_t *bps2 = (node_block_t *)nps2->value;
            ilist_t *a2;
            for (a2 = bps2->list->begin;a2 != bps2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->kind == NODE_KIND_PARAMETER)
                {
                    cnt2 += 1;
                    if (cnt2 > cnt1)
                    {
                        node_parameter_t *parameter2 = (node_parameter_t *)item2->value;
                        if (parameter2->value == NULL)
                        {
                            goto region_by_name_check;
                        }
                    }
                }
            }

            return 1;
            
            region_by_name_check:
            for (a1 = bps1->list->begin;a1 != bps1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_PARAMETER)
                {
                    node_parameter_t *parameter1 = (node_parameter_t *)item1->value;

                    int32_t found = 0;
                    node_block_t *bps2 = (node_block_t *)nps2->value;
                    ilist_t *a2;
                    for (a2 = bps2->list->begin;a2 != bps2->list->end;a2 = a2->next)
                    {
                        node_t *item2 = (node_t *)a2->value;
                        if (item2->kind == NODE_KIND_PARAMETER)
                        {
                            node_parameter_t *parameter2 = (node_parameter_t *)item2->value;
                            if (semantic_idcmp(parameter1->key, parameter2->key) == 1)
                            {
                                found = 1;
                                list_t *response1 = list_create();
                                if (response1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                
                                int32_t r1 = semantic_expression(program, parameter1->type, response1, SEMANTIC_FLAG_NONE);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 1)
                                {
                                    uint64_t cnt_response1 = 0;

                                    ilist_t *b1;
                                    for (b1 = response1->begin;b1 != response1->end;b1 = b1->next)
                                    {
                                        cnt_response1 += 1;

                                        node_t *item3 = (node_t *)b1->value;

                                        list_t *response2 = list_create();
                                        if (response2 == NULL)
                                        {
                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }
                                        
                                        int32_t r2 = semantic_expression(program, parameter2->type, response2, SEMANTIC_FLAG_NONE);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            uint64_t cnt_response2 = 0;

                                            ilist_t *b2;
                                            for (b2 = response2->begin;b2 != response2->end;b2 = b2->next)
                                            {
                                                cnt_response2 += 1;

                                                node_t *item4 = (node_t *)b2->value;
                                                
                                                int32_t r3 = semantic_equivalence(program, item3, item4);
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
                                            }

                                            if (cnt_response2 == 0)
                                            {
                                                node_t *key1 = parameter2->key;
                                                node_basic_t *key_string1 = key1->value;

                                                semantic_error(program, parameter2->key, "Reference: type of '%s' not found\n\tInternal:%s-%u", 
                                                    key_string1->value, __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }
                                        
                                        list_destroy(response2);
                                        break;
                                    }

                                    if (cnt_response1 == 0)
                                    {
                                        node_t *key1 = parameter1->key;
                                        node_basic_t *key_string1 = key1->value;

                                        semantic_error(program, parameter1->key, "Reference: type of '%s' not found\n\tInternal:%s-%u", 
                                            key_string1->value, __FILE__, __LINE__);
                                        return -1;
                                    }
                                }
                                list_destroy(response1);
                            }
                        }
                    }

                    if (found == 0)
                    {
                        if (parameter1->value == NULL)
                        {
                            return 0;
                        }
                    }
                }
            }
            

            bps2 = (node_block_t *)nps2->value;
            for (a2 = bps2->list->begin;a2 != bps2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->kind == NODE_KIND_PARAMETER)
                {
                    node_parameter_t *parameter2 = (node_parameter_t *)item2->value;

                    int32_t found = 0;
                    for (a1 = bps1->list->begin;a1 != bps1->list->end;a1 = a1->next)
                    {
                        node_t *item1 = (node_t *)a1->value;
                        if (item1->kind == NODE_KIND_PARAMETER)
                        {
                            node_parameter_t *parameter1 = (node_parameter_t *)item1->value;

                            if (semantic_idcmp(parameter1->key, parameter2->key) == 1)
                            {
                                found = 1;
                                list_t *response1 = list_create();
                                if (response1 == NULL)
                                {
                                    fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                    return -1;
                                }
                                
                                int32_t r1 = semantic_expression(program, parameter1->type, response1, SEMANTIC_FLAG_NONE);
                                if (r1 == -1)
                                {
                                    return -1;
                                }
                                else
                                if (r1 == 1)
                                {
                                    uint64_t cnt_response1 = 0;

                                    ilist_t *b1;
                                    for (b1 = response1->begin;b1 != response1->end;b1 = b1->next)
                                    {
                                        cnt_response1 += 1;

                                        node_t *item3 = (node_t *)b1->value;

                                        list_t *response2 = list_create();
                                        if (response2 == NULL)
                                        {
                                            fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
                                            return -1;
                                        }
                                        
                                        int32_t r2 = semantic_expression(program, parameter2->type, response2, SEMANTIC_FLAG_NONE);
                                        if (r2 == -1)
                                        {
                                            return -1;
                                        }
                                        else
                                        if (r2 == 1)
                                        {
                                            uint64_t cnt_response2 = 0;

                                            ilist_t *b2;
                                            for (b2 = response2->begin;b2 != response2->end;b2 = b2->next)
                                            {
                                                cnt_response2 += 1;

                                                node_t *item4 = (node_t *)b2->value;
                                                
                                                int32_t r3 = semantic_equivalence(program, item3, item4);
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
                                            }

                                            if (cnt_response2 == 0)
                                            {
                                                node_t *key1 = parameter2->key;
                                                node_basic_t *key_string1 = key1->value;

                                                semantic_error(program, parameter2->key, "Reference: type of '%s' not found\n\tInternal:%s-%u", 
                                                    key_string1->value, __FILE__, __LINE__);
                                                return -1;
                                            }
                                        }
                                        list_destroy(response2);
                                        break;
                                    }

                                    if (cnt_response1 == 0)
                                    {
                                        node_t *key1 = parameter1->key;
                                        node_basic_t *key_string1 = key1->value;

                                        semantic_error(program, parameter1->key, "Reference: type of '%s' not found\n\tInternal:%s-%u", 
                                            key_string1->value, __FILE__, __LINE__);
                                        return -1;
                                    }
                                }
                                list_destroy(response1);
                            }
                        }
                    }

                    if (found == 0)
                    {
                        if (parameter2->value == NULL)
                        {
                            return 0;
                        }
                    }
                }
            }

            return 1;
        }
    }

    return 0;
}

int32_t
semantic_eqaul_psas(program_t *program, node_t *nps1, node_t *nas2)
{
   if (nps1 == NULL)
    {
        if (nas2 == NULL)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        if (nas2 == NULL)
        {
            node_block_t *bps1 = (node_block_t *)nps1->value;
            ilist_t *a1;
            for (a1 = bps1->list->begin;a1 != bps1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_PARAMETER)
                {
                    node_parameter_t *parameter1 = (node_parameter_t *)item1->value;
                    if (parameter1->value == NULL)
                    {
                        return 0;
                    }
                }
            }
            return 1;
        }
        else
        {
            uint64_t cnt2 = 0;
			int32_t use_by_key = 0;

			list_t *repository1 = list_create();
			if (repository1 == NULL)
			{
				fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        		return -1;
			}
            
			node_block_t *bas2 = (node_block_t *)nas2->value;
            ilist_t *a2;
            for (a2 = bas2->list->begin;a2 != bas2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->kind == NODE_KIND_ARGUMENT)
                {
					uint64_t cnt1 = 0;
                    cnt2 += 1;
					node_argument_t *argument2 = (node_argument_t *)item2->value;

					node_block_t *bps1 = (node_block_t *)nps1->value;

					ilist_t *a1;
					for (a1 = bps1->list->begin;a1 != bps1->list->end;a1 = a1->next)
					{
						node_t *item1 = (node_t *)a1->value;
						if (item1->kind == NODE_KIND_PARAMETER)
						{
							cnt1 += 1;
							
							node_parameter_t *parameter1 = (node_parameter_t *)item1->value;

							if (argument2->value == NULL)
							{
								if (cnt1 < cnt2)
								{
									continue;
								}

								if (use_by_key == 1)
								{
									semantic_error(program, item1, "fields format");
									return -1;
								}

								ilist_t *que1 = list_rpush(repository1, item1);
								if (que1 == NULL)
								{
									fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
									return -1;
								}

								if (parameter1->type != NULL)
								{
									list_t *response1 = list_create();
									if (response1 == NULL)
									{
										fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
										return -1;
									}
									
									int32_t r1 = semantic_resolve(program, parameter1->type->parent, parameter1->type, response1, SEMANTIC_FLAG_NONE);
									if (r1 == -1)
									{
										return -1;
									}
									else
									if (r1 == 1)
									{
                                        uint64_t cnt_response1 = 0;

										ilist_t *b1;
										for (b1 = response1->begin;b1 != response1->end;b1 = b1->next)
										{
                                            cnt_response1 += 1;

											node_t *item3 = (node_t *)b1->value;

											list_t *response2 = list_create();
											if (response2 == NULL)
											{
												fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
												return -1;
											}
											
											int32_t r2 = semantic_expression(program, argument2->key, response2, SEMANTIC_FLAG_NONE);
											if (r2 == -1)
											{
												return -1;
											}
											else
											if (r2 == 1)
											{
                                                uint64_t cnt_response2 = 0;

												ilist_t *b2;
												for (b2 = response2->begin;b2 != response2->end;b2 = b2->next)
												{
                                                    cnt_response2 += 1;

													node_t *item4 = (node_t *)b2->value;
													if (item3->id != item4->id)
													{
														list_destroy(response2);
														list_destroy(response1);
														return 0;
													}
												}

                                                if (cnt_response2 == 0)
                                                {
                                                    semantic_error(program, argument2->key, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                                    return -1;
                                                }
											}
											list_destroy(response2);
										}

                                        if (cnt_response1 == 0)
                                        {
                                            semantic_error(program, parameter1->type, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                            return -1;
                                        }
									}
                                    list_destroy(response1);
								}
								break;
							}
							else
							{
								if (argument2->key->kind != NODE_KIND_ID)
								{
									semantic_error(program, item1, "not an key id");
									return -1;
								}
								use_by_key = 1;

								if (semantic_idcmp(argument2->key, parameter1->key) == 1)
								{
									ilist_t *que1 = list_rpush(repository1, item1);
									if (que1 == NULL)
									{
										fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
										return -1;
									}

									if (parameter1->type != NULL)
									{
										list_t *response1 = list_create();
										if (response1 == NULL)
										{
											fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
											return -1;
										}
										
										int32_t r1 = semantic_resolve(program, parameter1->type->parent, parameter1->type, response1, SEMANTIC_FLAG_NONE);
										if (r1 == -1)
										{
											return -1;
										}
										else
										if (r1 == 1)
										{
                                            uint64_t cnt_response1 = 0;

											ilist_t *b1;
											for (b1 = response1->begin;b1 != response1->end;b1 = b1->next)
											{
                                                cnt_response1 += 1;

												node_t *item3 = (node_t *)b1->value;

												list_t *response2 = list_create();
												if (response2 == NULL)
												{
													fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
													return -1;
												}
												
												int32_t r2 = semantic_expression(program, argument2->value, response2, SEMANTIC_FLAG_NONE);
												if (r2 == -1)
												{
													return -1;
												}
												else
												if (r2 == 1)
												{
                                                    uint64_t cnt_response2 = 0;

													ilist_t *b2;
													for (b2 = response2->begin;b2 != response2->end;b2 = b2->next)
													{
                                                        cnt_response2 += 1;

														node_t *item4 = (node_t *)b2->value;
														if (item3->id != item4->id)
														{
															list_destroy(response2);
															list_destroy(response1);
															return 0;
														}
													}

                                                    if (cnt_response2 == 0)
                                                    {
                                                        semantic_error(program, argument2->value, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                                        return -1;
                                                    }
												}
												list_destroy(response2);
											}

                                            if (cnt_response1 == 0)
                                            {
                                                semantic_error(program, parameter1->type, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                                return -1;
                                            }
										}
                                        list_destroy(response1);
									}
								}
								
								break;
							}
						}
					}
                }
            }

            node_block_t *bps1 = (node_block_t *)nps1->value;
            ilist_t *a1;
            for (a1 = bps1->list->begin;a1 != bps1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_PARAMETER)
                {
					node_parameter_t *parameter1 = (node_parameter_t *)item1->value;

					int32_t found = 0;
					ilist_t *b1;
					for (b1 = repository1->begin;b1 != repository1->end;b1 = b1->next)
					{
						node_t *item2 = (node_t *)b1->value;
						if (item2->kind == NODE_KIND_PARAMETER)
						{
							if (item1->id == item2->id)
							{
								found = 1;
							}
						}
					}

					if (found == 0)
					{
						if (parameter1->value == NULL)
						{
							list_destroy(repository1);
							return 0;
						}
					}
                }
            }

			list_destroy(repository1);
            return 1;
        }
    }
    return 0;
}

int32_t
semantic_eqaul_gsfs(program_t *program, node_t *ngs1, node_t *nfs2)
{
    if (ngs1 == NULL)
    {
        if (nfs2 == NULL)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        if (nfs2 == NULL)
        {
            node_block_t *bps1 = (node_block_t *)ngs1->value;
            ilist_t *a1;
            for (a1 = bps1->list->begin;a1 != bps1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_GENERIC)
                {
                    node_generic_t *generic1 = (node_generic_t *)item1->value;
                    if (generic1->value == NULL)
                    {
                        return 0;
                    }
                }
            }
            return 1;
        }
        else
        {
            uint64_t cnt2 = 0;
			int32_t use_by_key = 0;

			list_t *repository1 = list_create();
			if (repository1 == NULL)
			{
				fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        		return -1;
			}
            
			node_block_t *bfs2 = (node_block_t *)nfs2->value;
            ilist_t *a2;
            for (a2 = bfs2->list->begin;a2 != bfs2->list->end;a2 = a2->next)
            {
                node_t *item2 = (node_t *)a2->value;
                if (item2->kind == NODE_KIND_FIELD)
                {
					uint64_t cnt1 = 0;
                    cnt2 += 1;
					node_field_t *field2 = (node_field_t *)item2->value;

					node_block_t *bgs1 = (node_block_t *)ngs1->value;

					ilist_t *a1;
					for (a1 = bgs1->list->begin;a1 != bgs1->list->end;a1 = a1->next)
					{
						node_t *item1 = (node_t *)a1->value;
						if (item1->kind == NODE_KIND_GENERIC)
						{
							cnt1 += 1;
							
							node_generic_t *generic1 = (node_generic_t *)item1->value;

							if (field2->value == NULL)
							{
								if (cnt1 < cnt2)
								{
									continue;
								}

								if (use_by_key == 1)
								{
									semantic_error(program, item1, "fields format");
									return -1;
								}

								ilist_t *que1 = list_rpush(repository1, item1);
								if (que1 == NULL)
								{
									fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
									return -1;
								}

								if (generic1->type != NULL)
								{
									list_t *response1 = list_create();
									if (response1 == NULL)
									{
										fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
										return -1;
									}
									
									int32_t r1 = semantic_resolve(program, generic1->type->parent, generic1->type, response1, SEMANTIC_FLAG_NONE);
									if (r1 == -1)
									{
										return -1;
									}
									else
									if (r1 == 1)
									{
                                        uint64_t cnt_response1 = 0;

										ilist_t *b1;
										for (b1 = response1->begin;b1 != response1->end;b1 = b1->next)
										{
                                            cnt_response1 += 1;

											node_t *item3 = (node_t *)b1->value;

											list_t *response2 = list_create();
											if (response2 == NULL)
											{
												fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
												return -1;
											}
											
											int32_t r2 = semantic_resolve(program, field2->key->parent, field2->key, response2, SEMANTIC_FLAG_NONE);
											if (r2 == -1)
											{
												return -1;
											}
											else
											if (r2 == 1)
											{
                                                uint64_t cnt_response2 = 0;

												ilist_t *b2;
												for (b2 = response2->begin;b2 != response2->end;b2 = b2->next)
												{
                                                    cnt_response2 += 1;

													node_t *item4 = (node_t *)b2->value;
													int32_t r3 = semantic_subset(program, item3, item4);
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
												}

                                                if (cnt_response2 == 0)
                                                {
                                                    semantic_error(program, field2->key, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                                    return -1;
                                                }
											}
											list_destroy(response2);
										}
									
                                        if (cnt_response1 == 0)
                                        {
                                            semantic_error(program, generic1->type, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                            return -1;
                                        }
									}
                                    list_destroy(response1);
								}
								break;
							}
							else
							{
								if (field2->key->kind != NODE_KIND_ID)
								{
									semantic_error(program, item1, "not an key id");
									return -1;
								}
								use_by_key = 1;

								if (semantic_idcmp(field2->key, generic1->key) == 1)
								{
									ilist_t *que1 = list_rpush(repository1, item1);
									if (que1 == NULL)
									{
										fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
										return -1;
									}

									if (generic1->type != NULL)
									{
										list_t *response1 = list_create();
										if (response1 == NULL)
										{
											fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
											return -1;
										}
										
										int32_t r1 = semantic_resolve(program, generic1->type->parent, generic1->type, response1, SEMANTIC_FLAG_NONE);
										if (r1 == -1)
										{
											return -1;
										}
										else
										if (r1 == 1)
										{
                                            uint64_t cnt_response1 = 0;

											ilist_t *b1;
											for (b1 = response1->begin;b1 != response1->end;b1 = b1->next)
											{
                                                cnt_response1 += 1;

												node_t *item3 = (node_t *)b1->value;

												list_t *response2 = list_create();
												if (response2 == NULL)
												{
													fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
													return -1;
												}
												
												int32_t r2 = semantic_resolve(program, field2->value->parent, field2->value, response2, SEMANTIC_FLAG_NONE);
												if (r2 == -1)
												{
													return -1;
												}
												else
												if (r2 == 1)
												{
                                                    uint64_t cnt_response2 = 0;

													ilist_t *b2;
													for (b2 = response2->begin;b2 != response2->end;b2 = b2->next)
													{
                                                        cnt_response2 += 1;

														node_t *item4 = (node_t *)b2->value;
														int32_t r3 = semantic_subset(program, item3, item4);
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
													}

                                                    if (cnt_response2 == 0)
                                                    {
                                                        semantic_error(program, field2->value, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                                        return -1;
                                                    }
												}
												list_destroy(response2);
											}

                                            if (cnt_response1 == 0)
                                            {
                                                semantic_error(program, generic1->type, "Reference not found\n\tInternal:%s-%u", __FILE__, __LINE__);
                                                return -1;
                                            }
										}
                                        list_destroy(response1);
									}
								}

								
								break;
							}
						}
					}
                }
            }

            node_block_t *bgs1 = (node_block_t *)ngs1->value;
            ilist_t *a1;
            for (a1 = bgs1->list->begin;a1 != bgs1->list->end;a1 = a1->next)
            {
                node_t *item1 = (node_t *)a1->value;
                if (item1->kind == NODE_KIND_GENERIC)
                {
					node_generic_t *generic1 = (node_generic_t *)item1->value;

					int32_t found = 0;
					ilist_t *b1;
					for (b1 = repository1->begin;b1 != repository1->end;b1 = b1->next)
					{
						node_t *item2 = (node_t *)b1->value;
						if (item2->kind == NODE_KIND_GENERIC)
						{
							if (item1->id == item2->id)
							{
								found = 1;
							}
						}
					}

					if (found == 0)
					{
						if (generic1->value == NULL)
						{
							list_destroy(repository1);
							return 0;
						}
					}
                }
            }

			list_destroy(repository1);
            return 1;
        }
    }
    return 1;
}

int32_t
semantic_eqaul_psns(program_t *program, node_t *nps1, node_t *nns2)
{
    return 1;
}

int32_t
semantic_eqaul_otos(program_t *program, node_t *not1, node_t *nos1)
{
    return 1;
}

int32_t
semantic_eqaul_osos(program_t *program, node_t *nos1, node_t *nos2)
{
    return 1;
}