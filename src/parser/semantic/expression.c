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
    if (node->kind == NODE_KIND_ITEM)
    {
        return semantic_item(program, base, node, response, flag);
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

                                                ilist_t *il1 = list_rpush(response, item3);
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

                                                ilist_t *il1 = list_rpush(response, item3);
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
                                            node_t *key1 = generic1->key;
                                            node_basic_t *key_string1 = key1->value;

                                            semantic_error(program, generic1->type, "Reference:type of '%s' not found\n\tInternal:%s-%u", 
                                                key_string1->value, __FILE__, __LINE__);
                                            return -1;
                                        }
                                    }

                                    list_destroy(response3);
                                }
                                else
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

                        if (cnt_response2 == 0)
                        {
                            node_t *key1 = heritage1->key;
                            node_basic_t *key_string1 = key1->value;

                            semantic_error(program, heritage1->type, "Reference:typeof '%s' not found\n\tInternal:%s-%u", 
                                key_string1->value, __FILE__, __LINE__);
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
semantic_unary(program_t *program, node_t *node, char *OPERATOR_SIMBOL, int32_t (*f)(program_t *, node_t *, list_t *, uint64_t), list_t *response, uint64_t flag)
{
    semantic_error(program, node, "Typing:it is wrong to use operator '%s' in typing\n\tInternal:%s-%u", 
        OPERATOR_SIMBOL, __FILE__, __LINE__);
    return -1;
}

static int32_t
semantic_binary(program_t *program, node_t *node, char *OPERATOR_SIMBOL, int32_t (*f)(program_t *, node_t *, list_t *, uint64_t), list_t *response, uint64_t flag)
{
    semantic_error(program, node, "Typing:it is wrong to use operator '%s' in typing\n\tInternal:%s-%u", 
        OPERATOR_SIMBOL, __FILE__, __LINE__);
    return -1;
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
        return semantic_binary(program, node, "==", semantic_relational, response, flag);
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

int32_t
semantic_assign(program_t *program, node_t *node, uint64_t flag)
{
    return 1;
}
