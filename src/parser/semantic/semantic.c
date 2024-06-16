/*
 * Creator: Yasser Sajjadi(Ys)
 * Date: 24/3/2024
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#include <jansson.h>

#include "../../types/types.h"
#include "../../container/queue.h"
#include "../../token/position.h"
#include "../../token/token.h"
#include "../../scanner/scanner.h"
#include "../../ast/node.h"
#include "../../utils/utils.h"
#include "../../utils/path.h"
#include "../syntax/syntax.h"
#include "../../error.h"
#include "../../mutex.h"
#include "../../config.h"
#include "../../repository.h"

#include "semantic.h"

int32_t
not_semantic_idcmp(not_node_t *n1, not_node_t *n2)
{
    not_node_basic_t *nb1 = (not_node_basic_t *)n1->value;
    not_node_basic_t *nb2 = (not_node_basic_t *)n2->value;

    // printf("%s %s\n", nb1->value, nb2->value);

    return (strcmp(nb1->value, nb2->value) == 0);
}

int32_t
not_semantic_strcmp(not_node_t *n1, char *name)
{
    not_node_basic_t *nb1 = (not_node_basic_t *)n1->value;
    return (strcmp(nb1->value, name) == 0);
}

static int32_t
not_semantic_var(not_node_t *node);

static int32_t
not_semantic_parameter(not_node_t *node);

static int32_t
not_semantic_parameters(not_node_t *node);

static int32_t
not_semantic_arguments(not_node_t *node);

static int32_t
not_semantic_fields(not_node_t *node);

static int32_t
not_semantic_generics(not_node_t *node);

static int32_t
not_semantic_body(not_node_t *node);

static int32_t
not_semantic_expression(not_node_t *node);

static int32_t
not_semantic_assign(not_node_t *node);

static int32_t
not_semantic_id(not_node_t *node)
{
    return 0;
}

static int32_t
not_semantic_number(not_node_t *node)
{
    return 0;
}

static int32_t
not_semantic_char(not_node_t *node)
{
    return 0;
}

static int32_t
not_semantic_string(not_node_t *node)
{
    return 0;
}

static int32_t
not_semantic_null(not_node_t *node)
{
    return 0;
}

static int32_t
not_semantic_kint(not_node_t *node)
{
    return 0;
}

static int32_t
not_semantic_kfloat(not_node_t *node)
{
    return 0;
}

static int32_t
not_semantic_kchar(not_node_t *node)
{
    return 0;
}

static int32_t
not_semantic_kstring(not_node_t *node)
{
    return 0;
}

static int32_t
not_semantic_lambda(not_node_t *node)
{
    not_node_lambda_t *fun1 = (not_node_lambda_t *)node->value;

    if (fun1->generics != NULL)
    {
        int32_t r1 = not_semantic_generics(fun1->generics);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (fun1->parameters != NULL)
    {
        int32_t r1 = not_semantic_parameters(fun1->parameters);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (fun1->body != NULL)
    {
        if (fun1->body->kind == NODE_KIND_BODY)
        {
            int32_t r1 = not_semantic_body(fun1->body);
            if (r1 == -1)
            {
                return -1;
            }
        }
        else
        {
            int32_t r1 = not_semantic_expression(fun1->body);
            if (r1 == -1)
            {
                return -1;
            }
        }
    }

    return 0;
}

static int32_t
not_semantic_parenthesis(not_node_t *node)
{
    not_node_unary_t *unary1 = (not_node_unary_t *)node->value;

    int32_t r1 = not_semantic_expression(unary1->right);
    if (r1 == -1)
    {
        return -1;
    }

    return 0;
}

static int32_t
not_semantic_primary(not_node_t *node)
{
    if (node->kind == NODE_KIND_ID)
    {
        return not_semantic_id(node);
    }
    else

        if (node->kind == NODE_KIND_NUMBER)
    {
        return not_semantic_number(node);
    }
    else if (node->kind == NODE_KIND_CHAR)
    {
        return not_semantic_char(node);
    }
    else if (node->kind == NODE_KIND_STRING)
    {
        return not_semantic_string(node);
    }
    else

        if (node->kind == NODE_KIND_NULL)
    {
        return not_semantic_null(node);
    }
    else

        if (node->kind == NODE_KIND_KINT)
    {
        return not_semantic_kint(node);
    }
    else

        if (node->kind == NODE_KIND_KFLOAT)
    {
        return not_semantic_kfloat(node);
    }
    else

        if (node->kind == NODE_KIND_KCHAR)
    {
        return not_semantic_kchar(node);
    }
    else if (node->kind == NODE_KIND_KSTRING)
    {
        return not_semantic_kstring(node);
    }
    else

        if (node->kind == NODE_KIND_TUPLE)
    {
        // return semantic_tuple(node);
        return 0;
    }
    else if (node->kind == NODE_KIND_OBJECT)
    {
        // return semantic_object(node);
        return 0;
    }
    else

        if (node->kind == NODE_KIND_LAMBDA)
    {
        return not_semantic_lambda(node);
    }
    else if (node->kind == NODE_KIND_PARENTHESIS)
    {
        return not_semantic_parenthesis(node);
    }

    else
    {
        not_error_semantic_by_node(node, "not a primary type");
        return -1;
    }
}

static int32_t
not_semantic_call(not_node_t *node)
{
    not_node_carrier_t *carrier1 = (not_node_carrier_t *)node->value;

    int32_t r1 = not_semantic_expression(carrier1->base);
    if (r1 == -1)
    {
        return -1;
    }

    if (carrier1->data)
    {
        int32_t r2 = not_semantic_arguments(carrier1->data);
        if (r2 == -1)
        {
            return -1;
        }
    }

    return 0;
}

static int32_t
not_semantic_array(not_node_t *node)
{
    not_node_carrier_t *carrier1 = (not_node_carrier_t *)node->value;

    int32_t r1 = not_semantic_expression(carrier1->base);
    if (r1 == -1)
    {
        return -1;
    }

    if (carrier1->data != NULL)
    {
        int32_t r1 = not_semantic_arguments(carrier1->data);
        if (r1 == -1)
        {
            return -1;
        }
        return 0;
    }

    return 0;
}

static int32_t
not_semantic_attribute(not_node_t *node)
{
    not_node_binary_t *binary1 = (not_node_binary_t *)node->value;

    int32_t r1 = not_semantic_expression(binary1->left);
    if (r1 == -1)
    {
        return -1;
    }

    int32_t r2 = not_semantic_expression(binary1->right);
    if (r2 == -1)
    {
        return -1;
    }

    return 0;
}

static int32_t
not_semantic_pseudonym(not_node_t *node)
{
    not_node_carrier_t *carrier1 = (not_node_carrier_t *)node->value;

    int32_t r1 = not_semantic_expression(carrier1->base);
    if (r1 == -1)
    {
        return -1;
    }

    int32_t r2 = not_semantic_fields(carrier1->data);
    if (r2 == -1)
    {
        return -1;
    }

    return 0;
}

static int32_t
not_semantic_postfix(not_node_t *node)
{
    if (node->kind == NODE_KIND_CALL)
    {
        return not_semantic_call(node);
    }
    else if (node->kind == NODE_KIND_ARRAY)
    {
        return not_semantic_array(node);
    }
    else if (node->kind == NODE_KIND_ATTRIBUTE)
    {
        return not_semantic_attribute(node);
    }
    else if (node->kind == NODE_KIND_PSEUDONYM)
    {
        return not_semantic_pseudonym(node);
    }
    else
    {
        return not_semantic_primary(node);
    }
}

static int32_t
not_semantic_prefix(not_node_t *node)
{
    if (node->kind == NODE_KIND_TILDE)
    {
        not_node_unary_t *unary = (not_node_unary_t *)node->value;
        int32_t r1 = not_semantic_expression(unary->right);
        if (r1 == -1)
        {
            return -1;
        }
        return 0;
    }
    else if (node->kind == NODE_KIND_POS)
    {
        not_node_unary_t *unary = (not_node_unary_t *)node->value;
        int32_t r1 = not_semantic_expression(unary->right);
        if (r1 == -1)
        {
            return -1;
        }
        return 0;
    }
    else if (node->kind == NODE_KIND_NEG)
    {
        not_node_unary_t *unary = (not_node_unary_t *)node->value;
        int32_t r1 = not_semantic_expression(unary->right);
        if (r1 == -1)
        {
            return -1;
        }
        return 0;
    }
    else if (node->kind == NODE_KIND_NOT)
    {
        not_node_unary_t *unary = (not_node_unary_t *)node->value;
        int32_t r1 = not_semantic_expression(unary->right);
        if (r1 == -1)
        {
            return -1;
        }
        return 0;
    }
    else if (node->kind == NODE_KIND_SIZEOF)
    {
        not_node_unary_t *unary = (not_node_unary_t *)node->value;
        int32_t r1 = not_semantic_expression(unary->right);
        if (r1 == -1)
        {
            return -1;
        }
        return 0;
    }
    else if (node->kind == NODE_KIND_TYPEOF)
    {
        not_node_unary_t *unary = (not_node_unary_t *)node->value;
        int32_t r1 = not_semantic_expression(unary->right);
        if (r1 == -1)
        {
            return -1;
        }
        return 0;
    }
    else
    {
        return not_semantic_postfix(node);
    }
}

static int32_t
not_semantic_pow(not_node_t *node)
{
    if (node->kind == NODE_KIND_POW)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;

        if (not_semantic_pow(binary->left) < 0)
        {
            return -1;
        }

        if (not_semantic_prefix(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else
    {
        return not_semantic_prefix(node);
    }
}

static int32_t
not_semantic_multipicative(not_node_t *node)
{
    if (node->kind == NODE_KIND_MUL)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;

        if (not_semantic_multipicative(binary->left) < 0)
        {
            return -1;
        }

        if (not_semantic_pow(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else if (node->kind == NODE_KIND_DIV)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;

        if (not_semantic_multipicative(binary->left) < 0)
        {
            return -1;
        }

        if (not_semantic_pow(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else if (node->kind == NODE_KIND_MOD)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;

        if (not_semantic_multipicative(binary->left) < 0)
        {
            return -1;
        }

        if (not_semantic_pow(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else if (node->kind == NODE_KIND_EPI)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;

        if (not_semantic_multipicative(binary->left) < 0)
        {
            return -1;
        }

        if (not_semantic_pow(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else
    {
        return not_semantic_pow(node);
    }
}

static int32_t
not_semantic_addative(not_node_t *node)
{
    if (node->kind == NODE_KIND_PLUS)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;

        if (not_semantic_addative(binary->left) < 0)
        {
            return -1;
        }

        if (not_semantic_multipicative(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else if (node->kind == NODE_KIND_MINUS)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;

        if (not_semantic_addative(binary->left) < 0)
        {
            return -1;
        }

        if (not_semantic_multipicative(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else
    {
        return not_semantic_multipicative(node);
    }
}

static int32_t
not_semantic_shifting(not_node_t *node)
{
    if (node->kind == NODE_KIND_SHR)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;

        if (not_semantic_addative(binary->left) < 0)
        {
            return -1;
        }

        if (not_semantic_addative(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else if (node->kind == NODE_KIND_SHL)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;

        if (not_semantic_addative(binary->left) < 0)
        {
            return -1;
        }

        if (not_semantic_addative(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else
    {
        return not_semantic_addative(node);
    }
}

static int32_t
not_semantic_relational(not_node_t *node)
{
    if (node->kind == NODE_KIND_LT)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;

        if (not_semantic_shifting(binary->left) < 0)
        {
            return -1;
        }

        if (not_semantic_shifting(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else if (node->kind == NODE_KIND_LE)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;

        if (not_semantic_shifting(binary->left) < 0)
        {
            return -1;
        }

        if (not_semantic_shifting(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else if (node->kind == NODE_KIND_GT)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;

        if (not_semantic_shifting(binary->left) < 0)
        {
            return -1;
        }

        if (not_semantic_shifting(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else if (node->kind == NODE_KIND_GE)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;

        if (not_semantic_shifting(binary->left) < 0)
        {
            return -1;
        }

        if (not_semantic_shifting(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else
    {
        return not_semantic_shifting(node);
    }
}

static int32_t
not_semantic_equality(not_node_t *node)
{
    if (node->kind == NODE_KIND_EQ)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;

        if (not_semantic_relational(binary->left) < 0)
        {
            return -1;
        }

        if (not_semantic_relational(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else if (node->kind == NODE_KIND_NEQ)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;

        if (not_semantic_relational(binary->left) < 0)
        {
            return -1;
        }

        if (not_semantic_relational(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else
    {
        return not_semantic_relational(node);
    }
}

static int32_t
not_semantic_bitwise_and(not_node_t *node)
{
    if (node->kind == NODE_KIND_AND)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;

        if (not_semantic_bitwise_and(binary->left) < 0)
        {
            return -1;
        }

        if (not_semantic_equality(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else
    {
        return not_semantic_equality(node);
    }
}

static int32_t
not_semantic_bitwise_xor(not_node_t *node)
{
    if (node->kind == NODE_KIND_XOR)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;

        if (not_semantic_bitwise_xor(binary->left) < 0)
        {
            return -1;
        }

        if (not_semantic_bitwise_and(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else
    {
        return not_semantic_bitwise_and(node);
    }
}

static int32_t
not_semantic_bitwise_or(not_node_t *node)
{
    if (node->kind == NODE_KIND_OR)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;

        if (not_semantic_bitwise_or(binary->left) < 0)
        {
            return -1;
        }

        if (not_semantic_bitwise_xor(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else
    {
        return not_semantic_bitwise_xor(node);
    }
}

static int32_t
not_semantic_logical_and(not_node_t *node)
{
    if (node->kind == NODE_KIND_LAND)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;

        if (not_semantic_logical_and(binary->left) < 0)
        {
            return -1;
        }

        if (not_semantic_bitwise_or(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else
    {
        return not_semantic_bitwise_or(node);
    }
}

static int32_t
not_semantic_logical_or(not_node_t *node)
{
    if (node->kind == NODE_KIND_LOR)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;

        if (not_semantic_logical_or(binary->left) < 0)
        {
            return -1;
        }

        if (not_semantic_logical_and(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else
    {
        return not_semantic_logical_and(node);
    }
}

static int32_t
not_semantic_instanceof(not_node_t *node)
{
    if (node->kind == NODE_KIND_INSTANCEOF)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;

        if (not_semantic_logical_or(binary->left) < 0)
        {
            return -1;
        }

        if (not_semantic_logical_or(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else
    {
        return not_semantic_logical_or(node);
    }
}

static int32_t
not_semantic_conditional(not_node_t *node)
{
    if (node->kind == NODE_KIND_CONDITIONAL)
    {
        not_node_triple_t *triple = (not_node_triple_t *)node->value;

        if (not_semantic_instanceof(triple->base) < 0)
        {
            return -1;
        }

        if (not_semantic_conditional(triple->left) < 0)
        {
            return -1;
        }

        if (not_semantic_conditional(triple->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else
    {
        return not_semantic_instanceof(node);
    }
}

static int32_t
not_semantic_expression(not_node_t *node)
{
    return not_semantic_conditional(node);
}

static int32_t
not_semantic_assign(not_node_t *node)
{
    return 0;
}

static int32_t
not_semantic_if(not_node_t *node)
{
    not_node_if_t *if1 = (not_node_if_t *)node->value;

    if (if1->condition != NULL)
    {
        int32_t r1 = not_semantic_expression(if1->condition);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (if1->then_body != NULL)
    {
        int32_t r1 = not_semantic_body(if1->then_body);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (if1->else_body != NULL)
    {
        not_node_t *else_body1 = if1->else_body;

        if (else_body1->kind == NODE_KIND_IF)
        {
            int32_t r1 = not_semantic_if(if1->else_body);
            if (r1 == -1)
            {
                return -1;
            }
        }
        else
        {
            int32_t r1 = not_semantic_body(if1->else_body);
            if (r1 == -1)
            {
                return -1;
            }
        }
    }

    return 0;
}

static int32_t
not_semantic_for(not_node_t *node)
{
    not_node_for_t *for1 = (not_node_for_t *)node->value;

    if (for1->key != NULL)
    {
        for (not_node_t *node1 = node->parent, *subnode = node; node1 != NULL; subnode = node1, node1 = node1->parent)
        {
            if (node1->kind == NODE_KIND_CATCH)
            {
                not_node_catch_t *catch1 = (not_node_catch_t *)node1->value;

                for (not_node_t *item1 = catch1->parameters; item1 != NULL; item1 = item1->next)
                {
                    not_node_parameter_t *parameter1 = (not_node_parameter_t *)item1->value;
                    if (not_semantic_idcmp(for1->key, parameter1->key) == 1)
                    {
                        not_node_basic_t *basic1 = (not_node_basic_t *)for1->key->value;
                        not_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                   basic1->value, parameter1->key->position.line, parameter1->key->position.column);
                        return -1;
                    }
                }
                break;
            }
            else if (node1->kind == NODE_KIND_FOR)
            {
                not_node_for_t *for2 = (not_node_for_t *)node1->value;

                for (not_node_t *item1 = for2->initializer; item1 != NULL; item1 = item1->next)
                {
                    if (item1->kind == NODE_KIND_VAR)
                    {
                        not_node_var_t *var2 = (not_node_var_t *)item1->value;

                        if (var2->key->kind == NODE_KIND_ID)
                        {
                            if (not_semantic_idcmp(for1->key, var2->key) == 1)
                            {
                                not_node_basic_t *basic1 = (not_node_basic_t *)for1->key->value;

                                not_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                           basic1->value, var2->key->position.line, var2->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            for (not_node_t *item2 = var2->key; item2 != NULL; item2 = item2->next)
                            {
                                assert(item2->kind == NODE_KIND_ENTITY);

                                not_node_entity_t *entity1 = (not_node_entity_t *)item2->value;
                                if (not_semantic_idcmp(for1->key, entity1->key) == 1)
                                {
                                    not_node_basic_t *basic1 = (not_node_basic_t *)for1->key->value;

                                    not_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                               basic1->value, entity1->key->position.line, entity1->key->position.column);
                                    return -1;
                                }
                            }
                        }
                    }
                }
                break;
            }
            else if (node1->kind == NODE_KIND_FORIN)
            {
                not_node_forin_t *for2 = (not_node_forin_t *)node1->value;

                if (for2->field)
                {
                    if (not_semantic_idcmp(for1->key, for2->field) == 1)
                    {
                        not_node_basic_t *basic1 = (not_node_basic_t *)for1->key->value;

                        not_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                   basic1->value, for2->field->position.line, for2->field->position.column);
                        return -1;
                    }
                }

                if (for2->value)
                {
                    if (not_semantic_idcmp(for1->key, for2->value) == 1)
                    {
                        not_node_basic_t *basic1 = (not_node_basic_t *)for1->key->value;

                        not_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                   basic1->value, for2->value->position.line, for2->value->position.column);
                        return -1;
                    }
                }
            }
            else if (node1->kind == NODE_KIND_BODY)
            {
                not_node_body_t *body1 = (not_node_body_t *)node1->value;

                for (not_node_t *item1 = body1->declaration; item1 != NULL; item1 = item1->next)
                {
                    if (item1->id == subnode->id)
                    {
                        break;
                    }

                    if (item1->kind == NODE_KIND_FOR)
                    {
                        not_node_for_t *for2 = (not_node_for_t *)item1->value;
                        if (for2->key != NULL)
                        {
                            if (not_semantic_idcmp(for1->key, for2->key) == 1)
                            {
                                not_node_basic_t *basic1 = (not_node_basic_t *)for1->key->value;

                                not_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                           basic1->value, for2->key->position.line, for2->key->position.column);
                                return -1;
                            }
                        }
                    }
                    else if (item1->kind == NODE_KIND_FORIN)
                    {
                        not_node_forin_t *for2 = (not_node_forin_t *)item1->value;
                        if (for2->key != NULL)
                        {
                            if (not_semantic_idcmp(for1->key, for2->key) == 1)
                            {
                                not_node_basic_t *basic1 = (not_node_basic_t *)for1->key->value;

                                not_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                           basic1->value, for2->key->position.line, for2->key->position.column);
                                return -1;
                            }
                        }
                    }
                    else if (item1->kind == NODE_KIND_VAR)
                    {
                        not_node_var_t *var2 = (not_node_var_t *)item1->value;
                        if (var2->key->kind == NODE_KIND_ID)
                        {
                            if (not_semantic_idcmp(for1->key, var2->key) == 1)
                            {
                                not_node_basic_t *basic1 = (not_node_basic_t *)for1->key->value;

                                not_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                           basic1->value, var2->key->position.line, var2->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            not_node_block_t *block = (not_node_block_t *)var2->key->value;
                            for (not_node_t *item2 = block->items; item2 != NULL; item2 = item2->next)
                            {
                                assert(item2->kind == NODE_KIND_ENTITY);
                                not_node_entity_t *entity1 = (not_node_entity_t *)item2->value;
                                if (not_semantic_idcmp(for1->key, entity1->key) == 1)
                                {
                                    not_node_basic_t *basic1 = (not_node_basic_t *)for1->key->value;

                                    not_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                               basic1->value, entity1->key->position.line, entity1->key->position.column);
                                    return -1;
                                }
                            }
                        }
                    }
                }

                continue;
            }
            else if (node1->kind == NODE_KIND_FUN)
            {
                not_node_fun_t *fun2 = (not_node_fun_t *)node1->value;

                if (fun2->generics != NULL)
                {
                    for (not_node_t *item1 = fun2->generics; item1 != NULL; item1 = item1->next)
                    {
                        assert(item1->kind == NODE_KIND_GENERIC);

                        not_node_generic_t *generic1 = (not_node_generic_t *)item1->value;
                        if (not_semantic_idcmp(for1->key, generic1->key) == 1)
                        {
                            not_node_basic_t *basic1 = (not_node_basic_t *)for1->key->value;

                            not_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                       basic1->value, generic1->key->position.line, generic1->key->position.column);
                            return -1;
                        }
                    }
                }

                if (fun2->parameters != NULL)
                {
                    for (not_node_t *item2 = fun2->parameters; item2 != NULL; item2 = item2->next)
                    {
                        assert(item2->kind == NODE_KIND_PARAMETER);

                        not_node_parameter_t *parameter1 = (not_node_parameter_t *)item2->value;
                        if (not_semantic_idcmp(for1->key, parameter1->key) == 1)
                        {
                            not_node_basic_t *basic1 = (not_node_basic_t *)for1->key->value;

                            not_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                       basic1->value, parameter1->key->position.line, parameter1->key->position.column);
                            return -1;
                        }
                    }
                }
                break;
            }
            else if (node1->kind == NODE_KIND_MODULE)
            {
                not_node_block_t *module1 = (not_node_block_t *)node1->value;

                for (not_node_t *item1 = module1->items; item1 != NULL; item1 = item1->next)
                {
                    if (item1->id == subnode->id)
                    {
                        break;
                    }

                    if (item1->kind == NODE_KIND_USING)
                    {
                        not_node_using_t *using1 = (not_node_using_t *)item1->value;
                        not_node_block_t *packages1 = (not_node_block_t *)using1->packages->value;
                        for (not_node_t *item2 = packages1->items; item2 != NULL; item2 = item2->next)
                        {
                            assert(item2->kind == NODE_KIND_PACKAGE);

                            not_node_package_t *package2 = (not_node_package_t *)item2->value;
                            if (not_semantic_idcmp(for1->key, package2->key) == 1)
                            {
                                not_node_basic_t *basic1 = (not_node_basic_t *)for1->key->value;

                                not_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                           basic1->value, package2->key->position.line, package2->key->position.column);
                                return -1;
                            }
                        }
                    }
                    else if (item1->kind == NODE_KIND_CLASS)
                    {
                        not_node_class_t *class1 = (not_node_class_t *)item1->value;

                        if (not_semantic_idcmp(for1->key, class1->key) == 1)
                        {
                            not_node_basic_t *basic1 = (not_node_basic_t *)for1->key->value;

                            not_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                       basic1->value, class1->key->position.line, class1->key->position.column);
                            return -1;
                        }
                    }
                    else if (item1->kind == NODE_KIND_FOR)
                    {
                        not_node_for_t *for2 = (not_node_for_t *)item1->value;
                        if (for2->key != NULL)
                        {
                            if (not_semantic_idcmp(for1->key, for2->key) == 1)
                            {
                                not_node_basic_t *basic1 = (not_node_basic_t *)for1->key->value;

                                not_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                           basic1->value, for2->key->position.line, for2->key->position.column);
                                return -1;
                            }
                        }
                    }
                    else if (item1->kind == NODE_KIND_FORIN)
                    {
                        not_node_forin_t *for2 = (not_node_forin_t *)item1->value;
                        if (for2->key != NULL)
                        {
                            if (not_semantic_idcmp(for1->key, for2->key) == 1)
                            {
                                not_node_basic_t *basic1 = (not_node_basic_t *)for1->key->value;

                                not_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                           basic1->value, for2->key->position.line, for2->key->position.column);
                                return -1;
                            }
                        }
                    }
                    else if (item1->kind == NODE_KIND_VAR)
                    {
                        not_node_var_t *var2 = (not_node_var_t *)item1->value;
                        if (var2->key->kind == NODE_KIND_ID)
                        {
                            if (not_semantic_idcmp(for1->key, var2->key) == 1)
                            {
                                not_node_basic_t *basic1 = (not_node_basic_t *)for1->key->value;

                                not_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                           basic1->value, var2->key->position.line, var2->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            not_node_block_t *block = (not_node_block_t *)var2->key->value;
                            for (not_node_t *item2 = block->items; item2 != NULL; item2 = item2->next)
                            {
                                assert(item2->kind == NODE_KIND_ENTITY);
                                not_node_entity_t *entity1 = (not_node_entity_t *)item2->value;
                                if (not_semantic_idcmp(for1->key, entity1->key) == 1)
                                {
                                    not_node_basic_t *basic1 = (not_node_basic_t *)for1->key->value;

                                    not_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                               basic1->value, entity1->key->position.line, entity1->key->position.column);
                                    return -1;
                                }
                            }
                        }
                    }
                }

                break;
            }
        }
    }

    if (for1->initializer != NULL)
    {
        for (not_node_t *item1 = for1->initializer; item1 != NULL; item1 = item1->next)
        {
            if (item1->kind == NODE_KIND_VAR)
            {
                int32_t r1 = not_semantic_var(item1);
                if (r1 == -1)
                {
                    return -1;
                }
            }
            else
            {
                int32_t r1 = not_semantic_assign(item1);
                if (r1 == -1)
                {
                    return -1;
                }
            }
        }
    }

    if (for1->condition != NULL)
    {
        int32_t r1 = not_semantic_expression(for1->condition);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (for1->body != NULL)
    {
        int32_t r2 = not_semantic_body(for1->body);
        if (r2 == -1)
        {
            return -1;
        }
    }

    if (for1->incrementor != NULL)
    {
        for (not_node_t *item1 = for1->incrementor; item1 != NULL; item1 = item1->next)
        {
            int32_t r2 = not_semantic_assign(item1);
            if (r2 == -1)
            {
                return -1;
            }
        }
    }

    return 0;
}

static int32_t
not_semantic_forin(not_node_t *node)
{
    not_node_forin_t *for1 = (not_node_forin_t *)node->value;

    if (for1->key != NULL)
    {
        for (not_node_t *node1 = node->parent, *subnode = node; node1 != NULL; subnode = node1, node1 = node1->parent)
        {
            if (node1->kind == NODE_KIND_CATCH)
            {
                not_node_catch_t *catch1 = (not_node_catch_t *)node1->value;

                for (not_node_t *item1 = catch1->parameters; item1 != NULL; item1 = item1->next)
                {
                    not_node_parameter_t *parameter1 = (not_node_parameter_t *)item1->value;
                    if (not_semantic_idcmp(for1->key, parameter1->key) == 1)
                    {
                        not_node_basic_t *basic1 = (not_node_basic_t *)for1->key->value;
                        not_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                   basic1->value, parameter1->key->position.line, parameter1->key->position.column);
                        return -1;
                    }
                }
                break;
            }
            else if (node1->kind == NODE_KIND_FOR)
            {
                not_node_for_t *for2 = (not_node_for_t *)node1->value;

                for (not_node_t *item1 = for2->initializer; item1 != NULL; item1 = item1->next)
                {
                    if (item1->kind == NODE_KIND_VAR)
                    {
                        not_node_var_t *var2 = (not_node_var_t *)item1->value;

                        if (var2->key->kind == NODE_KIND_ID)
                        {
                            if (not_semantic_idcmp(for1->key, var2->key) == 1)
                            {
                                not_node_basic_t *basic1 = (not_node_basic_t *)for1->key->value;

                                not_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                           basic1->value, var2->key->position.line, var2->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            for (not_node_t *item2 = var2->key; item2 != NULL; item2 = item2->next)
                            {
                                assert(item2->kind == NODE_KIND_ENTITY);

                                not_node_entity_t *entity1 = (not_node_entity_t *)item2->value;
                                if (not_semantic_idcmp(for1->key, entity1->key) == 1)
                                {
                                    not_node_basic_t *basic1 = (not_node_basic_t *)for1->key->value;

                                    not_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                               basic1->value, entity1->key->position.line, entity1->key->position.column);
                                    return -1;
                                }
                            }
                        }
                    }
                }
                break;
            }
            else if (node1->kind == NODE_KIND_FORIN)
            {
                not_node_forin_t *for2 = (not_node_forin_t *)node1->value;

                if (for2->field)
                {
                    if (not_semantic_idcmp(for1->key, for2->field) == 1)
                    {
                        not_node_basic_t *basic1 = (not_node_basic_t *)for1->key->value;

                        not_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                   basic1->value, for2->field->position.line, for2->field->position.column);
                        return -1;
                    }
                }

                if (for2->value)
                {
                    if (not_semantic_idcmp(for1->key, for2->value) == 1)
                    {
                        not_node_basic_t *basic1 = (not_node_basic_t *)for1->key->value;

                        not_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                   basic1->value, for2->value->position.line, for2->value->position.column);
                        return -1;
                    }
                }
            }
            else if (node1->kind == NODE_KIND_BODY)
            {
                not_node_body_t *body1 = (not_node_body_t *)node1->value;

                for (not_node_t *item1 = body1->declaration; item1 != NULL; item1 = item1->next)
                {
                    if (item1->id == subnode->id)
                    {
                        break;
                    }

                    if (item1->kind == NODE_KIND_FOR)
                    {
                        not_node_for_t *for2 = (not_node_for_t *)item1->value;
                        if (for2->key != NULL)
                        {
                            if (not_semantic_idcmp(for1->key, for2->key) == 1)
                            {
                                not_node_basic_t *basic1 = (not_node_basic_t *)for1->key->value;

                                not_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                           basic1->value, for2->key->position.line, for2->key->position.column);
                                return -1;
                            }
                        }
                    }
                    else if (item1->kind == NODE_KIND_FORIN)
                    {
                        not_node_forin_t *for2 = (not_node_forin_t *)item1->value;
                        if (for2->key != NULL)
                        {
                            if (not_semantic_idcmp(for1->key, for2->key) == 1)
                            {
                                not_node_basic_t *basic1 = (not_node_basic_t *)for1->key->value;

                                not_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                           basic1->value, for2->key->position.line, for2->key->position.column);
                                return -1;
                            }
                        }
                    }
                    else if (item1->kind == NODE_KIND_VAR)
                    {
                        not_node_var_t *var2 = (not_node_var_t *)item1->value;
                        if (var2->key->kind == NODE_KIND_ID)
                        {
                            if (not_semantic_idcmp(for1->key, var2->key) == 1)
                            {
                                not_node_basic_t *basic1 = (not_node_basic_t *)for1->key->value;

                                not_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                           basic1->value, var2->key->position.line, var2->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            not_node_block_t *block = (not_node_block_t *)var2->key->value;
                            for (not_node_t *item2 = block->items; item2 != NULL; item2 = item2->next)
                            {
                                assert(item2->kind == NODE_KIND_ENTITY);
                                not_node_entity_t *entity1 = (not_node_entity_t *)item2->value;
                                if (not_semantic_idcmp(for1->key, entity1->key) == 1)
                                {
                                    not_node_basic_t *basic1 = (not_node_basic_t *)for1->key->value;

                                    not_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                               basic1->value, entity1->key->position.line, entity1->key->position.column);
                                    return -1;
                                }
                            }
                        }
                    }
                }

                continue;
            }
            else if (node1->kind == NODE_KIND_FUN)
            {
                not_node_fun_t *fun2 = (not_node_fun_t *)node1->value;

                if (fun2->generics != NULL)
                {
                    for (not_node_t *item1 = fun2->generics; item1 != NULL; item1 = item1->next)
                    {
                        assert(item1->kind == NODE_KIND_GENERIC);

                        not_node_generic_t *generic1 = (not_node_generic_t *)item1->value;
                        if (not_semantic_idcmp(for1->key, generic1->key) == 1)
                        {
                            not_node_basic_t *basic1 = (not_node_basic_t *)for1->key->value;

                            not_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                       basic1->value, generic1->key->position.line, generic1->key->position.column);
                            return -1;
                        }
                    }
                }

                if (fun2->parameters != NULL)
                {
                    for (not_node_t *item2 = fun2->parameters; item2 != NULL; item2 = item2->next)
                    {
                        assert(item2->kind == NODE_KIND_PARAMETER);

                        not_node_parameter_t *parameter1 = (not_node_parameter_t *)item2->value;
                        if (not_semantic_idcmp(for1->key, parameter1->key) == 1)
                        {
                            not_node_basic_t *basic1 = (not_node_basic_t *)for1->key->value;

                            not_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                       basic1->value, parameter1->key->position.line, parameter1->key->position.column);
                            return -1;
                        }
                    }
                }
                break;
            }
            else if (node1->kind == NODE_KIND_MODULE)
            {
                not_node_block_t *module1 = (not_node_block_t *)node1->value;

                for (not_node_t *item1 = module1->items; item1 != NULL; item1 = item1->next)
                {
                    if (item1->id == subnode->id)
                    {
                        break;
                    }

                    if (item1->kind == NODE_KIND_USING)
                    {
                        not_node_using_t *using1 = (not_node_using_t *)item1->value;
                        not_node_block_t *packages1 = (not_node_block_t *)using1->packages->value;
                        for (not_node_t *item2 = packages1->items; item2 != NULL; item2 = item2->next)
                        {
                            assert(item2->kind == NODE_KIND_PACKAGE);

                            not_node_package_t *package2 = (not_node_package_t *)item2->value;
                            if (not_semantic_idcmp(for1->key, package2->key) == 1)
                            {
                                not_node_basic_t *basic1 = (not_node_basic_t *)for1->key->value;

                                not_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                           basic1->value, package2->key->position.line, package2->key->position.column);
                                return -1;
                            }
                        }
                    }
                    else if (item1->kind == NODE_KIND_CLASS)
                    {
                        not_node_class_t *class1 = (not_node_class_t *)item1->value;

                        if (not_semantic_idcmp(for1->key, class1->key) == 1)
                        {
                            not_node_basic_t *basic1 = (not_node_basic_t *)for1->key->value;

                            not_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                       basic1->value, class1->key->position.line, class1->key->position.column);
                            return -1;
                        }
                    }
                    else if (item1->kind == NODE_KIND_FOR)
                    {
                        not_node_for_t *for2 = (not_node_for_t *)item1->value;
                        if (for2->key != NULL)
                        {
                            if (not_semantic_idcmp(for1->key, for2->key) == 1)
                            {
                                not_node_basic_t *basic1 = (not_node_basic_t *)for1->key->value;

                                not_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                           basic1->value, for2->key->position.line, for2->key->position.column);
                                return -1;
                            }
                        }
                    }
                    else if (item1->kind == NODE_KIND_FORIN)
                    {
                        not_node_forin_t *for2 = (not_node_forin_t *)item1->value;
                        if (for2->key != NULL)
                        {
                            if (not_semantic_idcmp(for1->key, for2->key) == 1)
                            {
                                not_node_basic_t *basic1 = (not_node_basic_t *)for1->key->value;

                                not_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                           basic1->value, for2->key->position.line, for2->key->position.column);
                                return -1;
                            }
                        }
                    }
                    else if (item1->kind == NODE_KIND_VAR)
                    {
                        not_node_var_t *var2 = (not_node_var_t *)item1->value;
                        if (var2->key->kind == NODE_KIND_ID)
                        {
                            if (not_semantic_idcmp(for1->key, var2->key) == 1)
                            {
                                not_node_basic_t *basic1 = (not_node_basic_t *)for1->key->value;

                                not_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                           basic1->value, var2->key->position.line, var2->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            not_node_block_t *block = (not_node_block_t *)var2->key->value;
                            for (not_node_t *item2 = block->items; item2 != NULL; item2 = item2->next)
                            {
                                assert(item2->kind == NODE_KIND_ENTITY);
                                not_node_entity_t *entity1 = (not_node_entity_t *)item2->value;
                                if (not_semantic_idcmp(for1->key, entity1->key) == 1)
                                {
                                    not_node_basic_t *basic1 = (not_node_basic_t *)for1->key->value;

                                    not_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                               basic1->value, entity1->key->position.line, entity1->key->position.column);
                                    return -1;
                                }
                            }
                        }
                    }
                }

                break;
            }
        }
    }

    if (for1->body != NULL)
    {
        int32_t r2 = not_semantic_body(for1->body);
        if (r2 == -1)
        {
            return -1;
        }
    }

    return 0;
}

static int32_t
not_semantic_entity(not_node_t *node)
{
    not_node_entity_t *entity1 = (not_node_entity_t *)node->value;

    for (not_node_t *item1 = node->previous; item1 != NULL; item1 = item1->previous)
    {
        assert(item1->kind == NODE_KIND_ENTITY);

        not_node_entity_t *entity2 = (not_node_entity_t *)item1->value;
        if (not_semantic_idcmp(entity1->key, entity2->key) == 1)
        {
            not_node_basic_t *basic1 = (not_node_basic_t *)entity1->key->value;

            not_error_semantic_by_node(entity1->key, "'%s' already defined, previous in (%lld:%lld)",
                                       basic1->value, entity2->key->position.line, entity2->key->position.column);
            return -1;
        }
    }

    return 0;
}

static int32_t
not_semantic_set(not_node_t *node)
{
    not_node_block_t *block = (not_node_block_t *)node->value;

    for (not_node_t *item = block->items; item != NULL; item = item->next)
    {
        int32_t r1 = not_semantic_entity(item);
        if (r1 == -1)
        {
            return -1;
        }
    }
    return 0;
}

static int32_t
not_semantic_var(not_node_t *node)
{
    not_node_var_t *var1 = (not_node_var_t *)node->value;

    for (not_node_t *node1 = node->parent, *subnode = node; node1 != NULL; subnode = node1, node1 = node1->parent)
    {
        if (node1->kind == NODE_KIND_CATCH)
        {
            not_node_catch_t *catch1 = (not_node_catch_t *)node1->value;

            for (not_node_t *item1 = catch1->parameters; item1 != NULL; item1 = item1->next)
            {
                not_node_parameter_t *parameter2 = (not_node_parameter_t *)item1->value;
                if (var1->key->kind == NODE_KIND_ID)
                {
                    if (not_semantic_idcmp(var1->key, parameter2->key) == 1)
                    {
                        not_node_basic_t *basic1 = (not_node_basic_t *)var1->key->value;

                        not_error_semantic_by_node(var1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                   basic1->value, parameter2->key->position.line, parameter2->key->position.column);
                        return -1;
                    }
                }
                else
                {
                    not_node_block_t *block = (not_node_block_t *)var1->key->value;
                    for (not_node_t *item2 = block->items; item2 != NULL; item2 = item2->next)
                    {
                        not_node_entity_t *entity1 = (not_node_entity_t *)item2->value;
                        if (not_semantic_idcmp(entity1->key, parameter2->key) == 1)
                        {
                            not_node_basic_t *basic1 = (not_node_basic_t *)entity1->key->value;

                            not_error_semantic_by_node(entity1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                       basic1->value, parameter2->key->position.line, parameter2->key->position.column);
                            return -1;
                        }
                    }
                }
            }

            break;
        }
        else if (node1->kind == NODE_KIND_FOR)
        {
            not_node_for_t *for1 = (not_node_for_t *)node1->value;

            for (not_node_t *item1 = for1->initializer; item1 != NULL; item1 = item1->next)
            {
                if (item1 == node)
                {
                    break;
                }

                if (item1->kind == NODE_KIND_VAR)
                {
                    not_node_var_t *var2 = (not_node_var_t *)item1->value;
                    if (var2->key->kind == NODE_KIND_ID)
                    {
                        if (var1->key->kind == NODE_KIND_ID)
                        {
                            if (not_semantic_idcmp(var1->key, var2->key) == 1)
                            {
                                not_node_basic_t *basic1 = (not_node_basic_t *)var1->key->value;

                                not_error_semantic_by_node(var1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                           basic1->value, var2->key->position.line, var2->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            for (not_node_t *item2 = var1->key; item2 != NULL; item2 = item2->next)
                            {
                                assert(item2->kind == NODE_KIND_ENTITY);

                                not_node_entity_t *entity1 = (not_node_entity_t *)item2->value;
                                if (not_semantic_idcmp(entity1->key, var2->key) == 1)
                                {
                                    not_node_basic_t *basic1 = (not_node_basic_t *)entity1->key->value;

                                    not_error_semantic_by_node(entity1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                               basic1->value, var2->key->position.line, var2->key->position.column);
                                    return -1;
                                }
                            }
                        }
                    }
                    else
                    {
                        not_node_block_t *block = (not_node_block_t *)var1->key->value;
                        for (not_node_t *item2 = block->items; item2 != NULL; item2 = item2->next)
                        {
                            assert(item2->kind == NODE_KIND_ENTITY);

                            not_node_entity_t *entity2 = (not_node_entity_t *)item2->value;
                            if (var1->key->kind == NODE_KIND_ID)
                            {
                                if (not_semantic_idcmp(var1->key, entity2->key) == 1)
                                {
                                    not_node_basic_t *basic1 = (not_node_basic_t *)var1->key->value;

                                    not_error_semantic_by_node(var1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                               basic1->value, entity2->key->position.path, entity2->key->position.line, entity2->key->position.column);
                                    return -1;
                                }
                            }
                            else
                            {
                                for (not_node_t *item3 = var1->key; item3 != NULL; item3 = item3->next)
                                {
                                    assert(item3->kind == NODE_KIND_ENTITY);

                                    not_node_entity_t *entity1 = (not_node_entity_t *)item3->value;
                                    if (not_semantic_idcmp(entity1->key, entity2->key) == 1)
                                    {
                                        not_node_basic_t *basic1 = (not_node_basic_t *)entity1->key->value;

                                        not_error_semantic_by_node(entity1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                                   basic1->value, entity2->key->position.path, entity2->key->position.line, entity2->key->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            break;
        }
        else if (node1->kind == NODE_KIND_BODY)
        {
            not_node_body_t *body1 = (not_node_body_t *)node1->value;

            for (not_node_t *item1 = body1->declaration; item1 != NULL; item1 = item1->next)
            {
                if (item1->id == subnode->id)
                {
                    break;
                }

                if (item1->kind == NODE_KIND_FOR)
                {
                    not_node_for_t *for2 = (not_node_for_t *)item1->value;
                    if (for2->key != NULL)
                    {
                        if (var1->key->kind == NODE_KIND_ID)
                        {
                            if (not_semantic_idcmp(var1->key, for2->key) == 1)
                            {
                                not_node_basic_t *basic1 = (not_node_basic_t *)var1->key->value;

                                not_error_semantic_by_node(var1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                           basic1->value, for2->key->position.line, for2->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            not_node_block_t *block = (not_node_block_t *)var1->key->value;
                            for (not_node_t *item2 = block->items; item2 != NULL; item2 = item2->next)
                            {
                                assert(item2->kind == NODE_KIND_ENTITY);

                                not_node_entity_t *entity1 = (not_node_entity_t *)item2->value;
                                if (not_semantic_idcmp(entity1->key, for2->key) == 1)
                                {
                                    not_node_basic_t *basic1 = (not_node_basic_t *)entity1->key->value;

                                    not_error_semantic_by_node(entity1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                               basic1->value, for2->key->position.line, for2->key->position.column);
                                    return -1;
                                }
                            }
                        }
                    }
                }
                else if (item1->kind == NODE_KIND_VAR)
                {
                    not_node_var_t *var2 = (not_node_var_t *)item1->value;
                    if (var2->key->kind == NODE_KIND_ID)
                    {
                        if (var1->key->kind == NODE_KIND_ID)
                        {
                            if (not_semantic_idcmp(var1->key, var2->key) == 1)
                            {
                                not_node_basic_t *basic1 = (not_node_basic_t *)var1->key->value;

                                not_error_semantic_by_node(var1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                           basic1->value, var2->key->position.line, var2->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            not_node_block_t *block = (not_node_block_t *)var1->key->value;
                            for (not_node_t *item2 = block->items; item2 != NULL; item2 = item2->next)
                            {
                                assert(item2->kind == NODE_KIND_ENTITY);

                                not_node_entity_t *entity1 = (not_node_entity_t *)item2->value;
                                if (not_semantic_idcmp(entity1->key, var2->key) == 1)
                                {
                                    not_node_basic_t *basic1 = (not_node_basic_t *)entity1->key->value;

                                    not_error_semantic_by_node(entity1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                               basic1->value, var2->key->position.line, var2->key->position.column);
                                    return -1;
                                }
                            }
                        }
                    }
                    else
                    {
                        if (var1->key->kind == NODE_KIND_ID)
                        {
                            not_node_block_t *block = (not_node_block_t *)var2->key->value;
                            for (not_node_t *item2 = block->items; item2 != NULL; item2 = item2->next)
                            {
                                assert(item2->kind == NODE_KIND_ENTITY);

                                not_node_entity_t *entity1 = (not_node_entity_t *)item2->value;
                                if (not_semantic_idcmp(var1->key, entity1->key) == 1)
                                {
                                    not_node_basic_t *basic1 = (not_node_basic_t *)var1->key->value;

                                    not_error_semantic_by_node(var1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                               basic1->value, entity1->key->position.line, entity1->key->position.column);
                                    return -1;
                                }
                            }
                        }
                        else
                        {
                            not_node_block_t *block1 = (not_node_block_t *)var1->key->value;
                            for (not_node_t *item2 = block1->items; item2 != NULL; item2 = item2->next)
                            {
                                not_node_entity_t *entity1 = (not_node_entity_t *)item2->value;

                                not_node_block_t *block2 = (not_node_block_t *)var2->key->value;
                                for (not_node_t *item3 = block2->items; item3 != NULL; item3 = item3->next)
                                {
                                    assert(item3->kind == NODE_KIND_ENTITY);

                                    not_node_entity_t *entity2 = (not_node_entity_t *)item3->value;
                                    if (not_semantic_idcmp(entity1->key, entity2->key) == 1)
                                    {
                                        not_node_basic_t *basic1 = (not_node_basic_t *)entity1->key->value;

                                        not_error_semantic_by_node(entity1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                                   basic1->value, entity2->key->position.path, entity2->key->position.line, entity2->key->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            continue;
        }
        else if (node1->kind == NODE_KIND_FUN)
        {
            not_node_fun_t *fun1 = (not_node_fun_t *)node1->value;

            if (fun1->generics != NULL)
            {
                for (not_node_t *item1 = fun1->generics; item1 != NULL; item1 = item1->next)
                {
                    not_node_generic_t *generic2 = (not_node_generic_t *)item1->value;
                    if (var1->key->kind == NODE_KIND_ID)
                    {
                        if (not_semantic_idcmp(var1->key, generic2->key) == 1)
                        {
                            not_node_basic_t *basic1 = (not_node_basic_t *)var1->key->value;

                            not_error_semantic_by_node(var1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                       basic1->value, generic2->key->position.line, generic2->key->position.column);
                            return -1;
                        }
                    }
                    else
                    {
                        not_node_block_t *block = (not_node_block_t *)var1->key->value;
                        for (not_node_t *item3 = block->items; item3 != NULL; item3 = item3->next)
                        {
                            not_node_entity_t *entity1 = (not_node_entity_t *)item3->value;
                            if (not_semantic_idcmp(entity1->key, generic2->key) == 1)
                            {
                                not_node_basic_t *basic1 = (not_node_basic_t *)entity1->key->value;

                                not_error_semantic_by_node(entity1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                           basic1->value, generic2->key->position.line, generic2->key->position.column);
                                return -1;
                            }
                        }
                    }
                }
            }

            if (fun1->parameters != NULL)
            {
                for (not_node_t *item1 = fun1->parameters; item1 != NULL; item1 = item1->next)
                {
                    not_node_parameter_t *parameter1 = (not_node_parameter_t *)item1->value;
                    if (var1->key->kind == NODE_KIND_ID)
                    {
                        if (not_semantic_idcmp(var1->key, parameter1->key) == 1)
                        {
                            not_node_basic_t *basic1 = (not_node_basic_t *)var1->key->value;

                            not_error_semantic_by_node(var1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                       basic1->value, parameter1->key->position.line, parameter1->key->position.column);
                            return -1;
                        }
                    }
                    else
                    {
                        not_node_block_t *block = (not_node_block_t *)var1->key->value;
                        for (not_node_t *item2 = block->items; item2 != NULL; item2 = item2->next)
                        {
                            assert(item2->kind == NODE_KIND_ENTITY);

                            not_node_entity_t *entity1 = (not_node_entity_t *)item2->value;
                            if (not_semantic_idcmp(entity1->key, parameter1->key) == 1)
                            {
                                not_node_basic_t *basic1 = (not_node_basic_t *)entity1->key->value;

                                not_error_semantic_by_node(entity1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                           basic1->value, parameter1->key->position.line, parameter1->key->position.column);
                                return -1;
                            }
                        }
                    }
                }
            }

            break;
        }
        else if (node1->kind == NODE_KIND_MODULE)
        {
            not_node_block_t *module1 = (not_node_block_t *)node1->value;
            for (not_node_t *item1 = module1->items; item1 != NULL; item1 = item1->next)
            {
                if (item1->id == subnode->id)
                {
                    break;
                }

                if (item1->kind == NODE_KIND_USING)
                {
                    not_node_using_t *using1 = (not_node_using_t *)item1->value;
                    not_node_block_t *packages1 = (not_node_block_t *)using1->packages->value;
                    for (not_node_t *item2 = packages1->items; item2 != NULL; item2 = item2->next)
                    {
                        not_node_package_t *package1 = (not_node_package_t *)item2->value;

                        if (var1->key->kind == NODE_KIND_ID)
                        {
                            if (not_semantic_idcmp(var1->key, package1->key) == 1)
                            {
                                not_node_basic_t *basic1 = (not_node_basic_t *)var1->key->value;

                                not_error_semantic_by_node(var1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                           basic1->value, package1->key->position.line, package1->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            not_node_block_t *block = (not_node_block_t *)var1->key->value;
                            for (not_node_t *item3 = block->items; item3 != NULL; item3 = item3->next)
                            {
                                not_node_entity_t *entity1 = (not_node_entity_t *)item3->value;

                                if (not_semantic_idcmp(entity1->key, package1->key) == 1)
                                {
                                    not_node_basic_t *basic1 = (not_node_basic_t *)entity1->key->value;

                                    not_error_semantic_by_node(entity1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                               basic1->value, package1->key->position.line, package1->key->position.column);
                                    return -1;
                                }
                            }
                        }
                    }
                }
                else if (item1->kind == NODE_KIND_CLASS)
                {
                    not_node_class_t *class1 = (not_node_class_t *)item1->value;
                    if (var1->key->kind == NODE_KIND_ID)
                    {
                        if (not_semantic_idcmp(var1->key, class1->key) == 1)
                        {
                            not_node_basic_t *basic1 = (not_node_basic_t *)var1->key->value;

                            not_error_semantic_by_node(var1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                       basic1->value, class1->key->position.line, class1->key->position.column);
                            return -1;
                        }
                    }
                    else
                    {
                        not_node_block_t *block = (not_node_block_t *)var1->key->value;
                        for (not_node_t *item2 = block->items; item2 != NULL; item2 = item2->next)
                        {
                            assert(item2->kind == NODE_KIND_ENTITY);

                            not_node_entity_t *entity1 = (not_node_entity_t *)item2->value;
                            if (not_semantic_idcmp(entity1->key, class1->key) == 1)
                            {
                                not_node_basic_t *basic1 = (not_node_basic_t *)entity1->key->value;

                                not_error_semantic_by_node(entity1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                           basic1->value, class1->key->position.line, class1->key->position.column);
                                return -1;
                            }
                        }
                    }
                }
                else if (item1->kind == NODE_KIND_FOR)
                {
                    not_node_for_t *for2 = (not_node_for_t *)item1->value;
                    if (for2->key != NULL)
                    {
                        if (var1->key->kind == NODE_KIND_ID)
                        {
                            if (not_semantic_idcmp(var1->key, for2->key) == 1)
                            {
                                not_node_basic_t *basic1 = (not_node_basic_t *)var1->key->value;

                                not_error_semantic_by_node(var1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                           basic1->value, for2->key->position.line, for2->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            not_node_block_t *block = (not_node_block_t *)var1->key->value;
                            for (not_node_t *item2 = block->items; item2 != NULL; item2 = item2->next)
                            {
                                assert(item2->kind == NODE_KIND_ENTITY);

                                not_node_entity_t *entity1 = (not_node_entity_t *)item2->value;
                                if (not_semantic_idcmp(entity1->key, for2->key) == 1)
                                {
                                    not_node_basic_t *basic1 = (not_node_basic_t *)entity1->key->value;

                                    not_error_semantic_by_node(entity1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                               basic1->value, for2->key->position.line, for2->key->position.column);
                                    return -1;
                                }
                            }
                        }
                    }
                }
                else if (item1->kind == NODE_KIND_VAR)
                {
                    not_node_var_t *var2 = (not_node_var_t *)item1->value;
                    if (var2->key->kind == NODE_KIND_ID)
                    {
                        if (var1->key->kind == NODE_KIND_ID)
                        {
                            if (not_semantic_idcmp(var1->key, var2->key) == 1)
                            {
                                not_node_basic_t *basic1 = (not_node_basic_t *)var1->key->value;

                                not_error_semantic_by_node(var1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                           basic1->value, var2->key->position.line, var2->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            not_node_block_t *block = (not_node_block_t *)var1->key->value;
                            for (not_node_t *item2 = block->items; item2 != NULL; item2 = item2->next)
                            {
                                not_node_entity_t *entity1 = (not_node_entity_t *)item2->value;
                                if (not_semantic_idcmp(entity1->key, var2->key) == 1)
                                {
                                    not_node_basic_t *basic1 = (not_node_basic_t *)entity1->key->value;

                                    not_error_semantic_by_node(entity1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                               basic1->value, var2->key->position.line, var2->key->position.column);
                                    return -1;
                                }
                            }
                        }
                    }
                    else
                    {
                        if (var1->key->kind == NODE_KIND_ID)
                        {
                            not_node_block_t *block = (not_node_block_t *)var2->key->value;
                            for (not_node_t *item2 = block->items; item2 != NULL; item2 = item2->next)
                            {
                                not_node_entity_t *entity1 = (not_node_entity_t *)item2->value;
                                if (not_semantic_idcmp(var1->key, entity1->key) == 1)
                                {
                                    not_node_basic_t *basic1 = (not_node_basic_t *)var1->key->value;

                                    not_error_semantic_by_node(var1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                               basic1->value, entity1->key->position.line, entity1->key->position.column);
                                    return -1;
                                }
                            }
                        }
                        else
                        {
                            not_node_block_t *block1 = (not_node_block_t *)var1->key->value;
                            for (not_node_t *item2 = block1->items; item2 != NULL; item2 = item2->next)
                            {
                                not_node_entity_t *entity1 = (not_node_entity_t *)item2->value;
                                not_node_block_t *block2 = (not_node_block_t *)var2->key->value;
                                for (not_node_t *item3 = block2->items; item3 != NULL; item3 = item3->next)
                                {
                                    not_node_entity_t *entity2 = (not_node_entity_t *)item3->value;
                                    if (not_semantic_idcmp(entity1->key, entity2->key) == 1)
                                    {
                                        not_node_basic_t *basic1 = (not_node_basic_t *)entity1->key->value;

                                        not_error_semantic_by_node(entity1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                                   basic1->value, entity2->key->position.path, entity2->key->position.line, entity2->key->position.column);
                                        return -1;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            break;
        }
    }

    if (var1->key->kind == NODE_KIND_SET)
    {
        int32_t r1 = not_semantic_set(var1->key);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (var1->type != NULL)
    {
        int32_t r1 = not_semantic_expression(var1->type);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (var1->value != NULL)
    {
        int32_t r1 = not_semantic_expression(var1->value);
        if (r1 == -1)
        {
            return -1;
        }
    }

    return 0;
}

static int32_t
not_semantic_field(not_node_t *node)
{
    not_node_field_t *field1 = (not_node_field_t *)node->value;

    if (field1->value != NULL)
    {
        if (field1->key->kind != NODE_KIND_ID)
        {
            not_error_semantic_by_node(field1->key, "'%s' not a name", "Key");
            return -1;
        }
    }

    for (not_node_t *item1 = node->previous; item1 != NULL; item1 = item1->previous)
    {
        if (field1->value != NULL)
        {
            not_node_field_t *field2 = (not_node_field_t *)item1->value;
            if (field2->value != NULL)
            {
                if (not_semantic_idcmp(field1->key, field2->key) == 1)
                {
                    not_node_basic_t *key_string1 = field1->key->value;
                    not_error_semantic_by_node(field1->key, "'%s' already defined, previous in (%lld:%lld)",
                                               key_string1->value, field2->key->position.line, field2->key->position.column);
                    return -1;
                }
            }
        }
        else
        {
            not_node_field_t *field2 = (not_node_field_t *)item1->value;
            if (field2->value != NULL)
            {
                not_error_semantic_by_node(field1->key, "the single field is used after the pair field");
                return -1;
            }
        }
    }

    if (field1->key != NULL)
    {
        int32_t r1 = not_semantic_expression(field1->key);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (field1->value != NULL)
    {
        int32_t r1 = not_semantic_expression(field1->value);
        if (r1 == -1)
        {
            return -1;
        }
    }

    return 0;
}

static int32_t
not_semantic_fields(not_node_t *node)
{
    not_node_block_t *block = (not_node_block_t *)node->value;

    for (not_node_t *item = block->items; item != NULL; item = item->next)
    {
        int32_t r1 = not_semantic_field(item);
        if (r1 == -1)
        {
            return -1;
        }
    }
    return 0;
}

static int32_t
not_semantic_argument(not_node_t *node)
{
    not_node_argument_t *argument1 = (not_node_argument_t *)node->value;

    if (argument1->value != NULL)
    {
        if (argument1->key->kind != NODE_KIND_ID)
        {
            not_error_semantic_by_node(argument1->key, "'%s' not a name",
                                       "Key");
            return -1;
        }
    }

    for (not_node_t *item1 = node->previous; item1 != NULL; item1 = item1->previous)
    {
        not_node_argument_t *argument2 = (not_node_argument_t *)item1->value;

        if ((argument1->value != NULL) && (argument2->value != NULL))
        {
            if (not_semantic_idcmp(argument1->key, argument2->key) == 1)
            {
                not_node_basic_t *basic1 = (not_node_basic_t *)argument1->key->value;

                not_error_semantic_by_node(argument1->key, "'%s' already defined, previous in (%lld:%lld)",
                                           basic1->value, argument2->key->position.line, argument2->key->position.column);
                return -1;
            }
        }
    }

    if (argument1->key != NULL)
    {
        int32_t r1 = not_semantic_expression(argument1->key);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (argument1->value != NULL)
    {
        int32_t r1 = not_semantic_expression(argument1->value);
        if (r1 == -1)
        {
            return -1;
        }
    }

    return 0;
}

static int32_t
not_semantic_arguments(not_node_t *node)
{
    not_node_block_t *block = (not_node_block_t *)node->value;
    for (not_node_t *item = block->items; item != NULL; item = item->next)
    {
        int32_t r1 = not_semantic_argument(item);
        if (r1 == -1)
        {
            return -1;
        }
    }
    return 0;
}

static int32_t
not_semantic_parameter(not_node_t *node)
{
    not_node_parameter_t *parameter1 = (not_node_parameter_t *)node->value;

    for (not_node_t *item1 = node->previous; item1 != NULL; item1 = item1->previous)
    {
        assert(item1->kind == NODE_KIND_GENERIC);

        not_node_parameter_t *parameter2 = (not_node_parameter_t *)item1->value;
        if (not_semantic_idcmp(parameter1->key, parameter2->key) == 1)
        {
            not_node_basic_t *basic1 = (not_node_basic_t *)parameter1->key->value;

            not_error_semantic_by_node(parameter1->key, "'%s' already defined, previous in (%lld:%lld)",
                                       basic1->value, parameter2->key->position.line, parameter2->key->position.column);
            return -1;
        }
    }

    for (not_node_t *node1 = node->parent; node1 != NULL; node1 = node1->parent)
    {
        if (node1->kind == NODE_KIND_FUN)
        {
            not_node_fun_t *fun2 = (not_node_fun_t *)node1->value;

            if (fun2->generics != NULL)
            {
                not_node_block_t *block1 = (not_node_block_t *)fun2->generics->value;

                for (not_node_t *item1 = block1->items; item1 != NULL; item1 = item1->next)
                {
                    assert(item1->kind == NODE_KIND_GENERIC);

                    not_node_generic_t *generic1 = (not_node_generic_t *)item1->value;
                    if (not_semantic_idcmp(parameter1->key, generic1->key) == 1)
                    {
                        not_node_basic_t *basic1 = (not_node_basic_t *)parameter1->key->value;
                        not_error_semantic_by_node(parameter1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                   basic1->value, generic1->key->position.line, generic1->key->position.column);
                        return -1;
                    }
                }
            }
            break;
        }
    }

    if (parameter1->type != NULL)
    {
        int32_t r1 = not_semantic_expression(parameter1->type);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (parameter1->value != NULL)
    {
        int32_t r1 = not_semantic_expression(parameter1->value);
        if (r1 == -1)
        {
            return -1;
        }
    }

    return 0;
}

static int32_t
not_semantic_parameters(not_node_t *node)
{
    not_node_block_t *block = (not_node_block_t *)node->value;

    for (not_node_t *item = block->items; item != NULL; item = item->next)
    {
        int32_t r1 = not_semantic_parameter(item);
        if (r1 == -1)
        {
            return -1;
        }
    }
    return 0;
}

static int32_t
not_semantic_generic(not_node_t *node)
{
    not_node_generic_t *generic1 = (not_node_generic_t *)node->value;

    for (not_node_t *item1 = node->previous; item1 != NULL; item1 = item1->previous)
    {
        assert(item1->kind == NODE_KIND_GENERIC);

        not_node_generic_t *generic2 = (not_node_generic_t *)item1->value;
        if (not_semantic_idcmp(generic1->key, generic2->key) == 1)
        {
            not_node_basic_t *basic1 = (not_node_basic_t *)generic1->key->value;

            not_error_semantic_by_node(generic1->key, "'%s' already defined, previous in (%lld:%lld)",
                                       basic1->value, generic2->key->position.line, generic2->key->position.column);
            return -1;
        }
    }

    if (generic1->key != NULL)
    {
        int32_t r1 = not_semantic_expression(generic1->key);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (generic1->type != NULL)
    {
        int32_t r1 = not_semantic_expression(generic1->type);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (generic1->value != NULL)
    {
        int32_t r1 = not_semantic_expression(generic1->value);
        if (r1 == -1)
        {
            return -1;
        }
    }

    return 0;
}

static int32_t
not_semantic_generics(not_node_t *node)
{
    not_node_block_t *block = (not_node_block_t *)node->value;

    for (not_node_t *item = block->items; item != NULL; item = item->next)
    {
        int32_t r1 = not_semantic_generic(item);
        if (r1 == -1)
        {
            return -1;
        }
    }
    return 0;
}

static int32_t
not_semantic_catch(not_node_t *node)
{
    not_node_catch_t *catch1 = (not_node_catch_t *)node->value;

    int32_t r1 = not_semantic_parameters(catch1->parameters);
    if (r1 == -1)
    {
        return -1;
    }

    if (catch1->body != NULL)
    {
        int32_t r1 = not_semantic_body(catch1->body);
        if (r1 == -1)
        {
            return -1;
        }
    }

    return 0;
}

static int32_t
not_semantic_try(not_node_t *node)
{
    not_node_try_t *try1 = (not_node_try_t *)node->value;

    if (try1->body != NULL)
    {
        int32_t r1 = not_semantic_body(try1->body);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (try1->catchs != NULL)
    {
        int32_t r1 = not_semantic_catch(try1->catchs);
        if (r1 == -1)
        {
            return -1;
        }
    }

    return 0;
}

static int32_t
not_semantic_throw(not_node_t *node)
{
    return 0;
}

static int32_t
not_semantic_return(not_node_t *node)
{
    return 0;
}

static int32_t
not_semantic_continue(not_node_t *node)
{
    return 0;
}

static int32_t
not_semantic_break(not_node_t *node)
{
    return 0;
}

static int32_t
not_semantic_statement(not_node_t *node)
{
    if (node->kind == NODE_KIND_IF)
    {
        int32_t result = not_semantic_if(node);
        if (result == -1)
        {
            return -1;
        }
    }
    else if (node->kind == NODE_KIND_FOR)
    {
        int32_t result = not_semantic_for(node);
        if (result == -1)
        {
            return -1;
        }
    }
    else if (node->kind == NODE_KIND_FORIN)
    {
        int32_t result = not_semantic_forin(node);
        if (result == -1)
        {
            return -1;
        }
    }
    else if (node->kind == NODE_KIND_TRY)
    {
        int32_t result = not_semantic_try(node);
        if (result == -1)
        {
            return -1;
        }
    }
    else if (node->kind == NODE_KIND_VAR)
    {
        int32_t result = not_semantic_var(node);
        if (result == -1)
        {
            return -1;
        }
    }
    else if (node->kind == NODE_KIND_THROW)
    {
        int32_t result = not_semantic_throw(node);
        if (result == -1)
        {
            return -1;
        }
    }
    else if (node->kind == NODE_KIND_RETURN)
    {
        int32_t result = not_semantic_return(node);
        if (result == -1)
        {
            return -1;
        }
    }
    else if (node->kind == NODE_KIND_CONTINUE)
    {
        int32_t result = not_semantic_continue(node);
        if (result == -1)
        {
            return -1;
        }
    }
    else if (node->kind == NODE_KIND_BREAK)
    {
        int32_t result = not_semantic_break(node);
        if (result == -1)
        {
            return -1;
        }
    }
    else
    {
        return not_semantic_assign(node);
    }
    return 0;
}

static int32_t
not_semantic_body(not_node_t *node)
{
    for (not_node_t *item = node; item != NULL; item = item->next)
    {
        int32_t r1 = not_semantic_statement(item);
        if (r1 == -1)
        {
            return -1;
        }
    }
    return 0;
}

static int32_t
not_semantic_note(not_node_t *node)
{
    not_node_carrier_t *carrier1 = (not_node_carrier_t *)node->value;

    assert(carrier1->base->kind == NODE_KIND_ID);

    for (not_node_t *node1 = node->previous; node1 != NULL; node1 = node1->previous)
    {
        not_node_carrier_t *carrier2 = (not_node_carrier_t *)node1->value;

        assert(carrier2->base->kind == NODE_KIND_ID);

        if (not_semantic_idcmp(carrier1->base, carrier2->base) == 1)
        {
            not_node_basic_t *basic1 = (not_node_basic_t *)carrier1->base->value;

            not_error_semantic_by_node(carrier1->base, "'%s' already defined, previous in (%lld:%lld)",
                                       basic1->value, carrier2->base->position.line, carrier2->base->position.column);
            return -1;
        }
    }

    if (carrier1->base != NULL)
    {
        int32_t r1 = not_semantic_expression(carrier1->base);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (carrier1->data != NULL)
    {
        int32_t r1 = not_semantic_arguments(carrier1->data);
        if (r1 == -1)
        {
            return -1;
        }
    }

    return 0;
}

static int32_t
not_semantic_notes(not_node_t *node)
{
    not_node_block_t *block = (not_node_block_t *)node->value;

    for (not_node_t *item = block->items; item != NULL; item = item->next)
    {
        int32_t r1 = not_semantic_note(item);
        if (r1 == -1)
        {
            return -1;
        }
    }
    return 0;
}

static int32_t
not_semantic_fun(not_node_t *node)
{
    not_node_fun_t *fun1 = (not_node_fun_t *)node->value;

    for (not_node_t *node1 = node->parent; node1 != NULL; node1 = node1->parent)
    {
        if (node1->kind == NODE_KIND_CLASS)
        {
            not_node_class_t *class1 = (not_node_class_t *)node1->value;

            if (class1->generics != NULL)
            {
                not_node_block_t *block1 = (not_node_block_t *)class1->generics->value;

                for (not_node_t *item1 = block1->items; item1 != NULL; item1 = item1->next)
                {
                    assert(item1->kind == NODE_KIND_GENERIC);

                    not_node_generic_t *generic1 = (not_node_generic_t *)item1->value;
                    if (not_semantic_idcmp(fun1->key, generic1->key) == 1)
                    {
                        not_node_basic_t *basic1 = (not_node_basic_t *)fun1->key->value;

                        not_error_semantic_by_node(fun1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                   basic1->value, generic1->key->position.line, generic1->key->position.column);
                        return -1;
                    }
                }
            }

            if (class1->heritages != NULL)
            {
                not_node_block_t *block1 = (not_node_block_t *)class1->heritages->value;

                for (not_node_t *item1 = block1->items; item1 != NULL; item1 = item1->next)
                {
                    assert(item1->kind == NODE_KIND_HERITAGE);

                    not_node_heritage_t *heritage1 = (not_node_heritage_t *)item1->value;

                    if (not_semantic_idcmp(fun1->key, heritage1->key) == 1)
                    {
                        not_node_basic_t *basic1 = (not_node_basic_t *)fun1->key->value;

                        not_error_semantic_by_node(fun1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                   basic1->value, heritage1->key->position.line, heritage1->key->position.column);
                        return -1;
                    }
                }
            }

            for (not_node_t *item1 = class1->block; item1 != NULL; item1 = item1->next)
            {
                if (item1->id == node->id)
                {
                    break;
                }

                if (item1->kind == NODE_KIND_CLASS)
                {
                    not_node_class_t *class2 = (not_node_class_t *)item1->value;
                    if (not_semantic_idcmp(fun1->key, class2->key) == 1)
                    {
                        not_node_basic_t *basic1 = (not_node_basic_t *)fun1->key->value;

                        not_error_semantic_by_node(fun1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                   basic1->value, class2->key->position.line, class2->key->position.column);
                        return -1;
                    }
                }
                else if (item1->kind == NODE_KIND_FUN)
                {
                    not_node_fun_t *fun2 = (not_node_fun_t *)item1->value;

                    if (not_semantic_idcmp(fun1->key, fun2->key) == 1)
                    {
                        not_node_basic_t *basic1 = (not_node_basic_t *)fun1->key->value;
                        not_error_semantic_by_node(fun1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                   basic1->value, fun2->key->position.line, fun2->key->position.column);
                        return -1;
                    }
                }
                else if (item1->kind == NODE_KIND_PROPERTY)
                {
                    not_node_property_t *property1 = (not_node_property_t *)item1->value;
                    if (not_semantic_idcmp(fun1->key, property1->key) == 1)
                    {
                        not_node_basic_t *basic1 = (not_node_basic_t *)fun1->key->value;
                        not_error_semantic_by_node(fun1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                   basic1->value, property1->key->position.line, property1->key->position.column);
                        return -1;
                    }
                }
            }

            break;
        }
    }

    if (fun1->notes != NULL)
    {
        int32_t r1 = not_semantic_notes(fun1->notes);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (fun1->generics != NULL)
    {
        int32_t r1 = not_semantic_generics(fun1->generics);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (fun1->parameters != NULL)
    {
        int32_t r1 = not_semantic_parameters(fun1->parameters);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (fun1->body != NULL)
    {
        int32_t r1 = not_semantic_body(fun1->body);
        if (r1 == -1)
        {
            return -1;
        }
    }

    return 0;
}

static int32_t
not_semantic_property(not_node_t *node)
{
    not_node_property_t *property1 = (not_node_property_t *)node->value;

    for (not_node_t *node1 = node->parent; node1 != NULL; node1 = node1->parent)
    {
        if (node1->kind == NODE_KIND_CLASS)
        {
            not_node_class_t *class1 = (not_node_class_t *)node1->value;

            if (class1->generics != NULL)
            {
                not_node_block_t *block1 = (not_node_block_t *)class1->generics->value;

                for (not_node_t *item1 = block1->items; item1 != NULL; item1 = item1->next)
                {
                    assert(item1->kind == NODE_KIND_GENERIC);

                    not_node_generic_t *generic1 = (not_node_generic_t *)item1->value;
                    if (not_semantic_idcmp(property1->key, generic1->key) == 1)
                    {
                        not_node_basic_t *basic1 = (not_node_basic_t *)property1->key->value;

                        not_error_semantic_by_node(property1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                   basic1->value, generic1->key->position.line, generic1->key->position.column);
                        return -1;
                    }
                }
            }

            if (class1->heritages != NULL)
            {
                not_node_block_t *block1 = (not_node_block_t *)class1->heritages->value;

                for (not_node_t *item1 = block1->items; item1 != NULL; item1 = item1->next)
                {
                    assert(item1->kind == NODE_KIND_HERITAGE);

                    not_node_heritage_t *heritage1 = (not_node_heritage_t *)item1->value;

                    if (not_semantic_idcmp(property1->key, heritage1->key) == 1)
                    {
                        not_node_basic_t *basic1 = (not_node_basic_t *)property1->key->value;

                        not_error_semantic_by_node(property1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                   basic1->value, heritage1->key->position.line, heritage1->key->position.column);
                        return -1;
                    }
                }
            }

            for (not_node_t *item1 = class1->block; item1 != NULL; item1 = item1->next)
            {
                if (item1->id == node->id)
                {
                    break;
                }

                if (item1->kind == NODE_KIND_CLASS)
                {
                    not_node_class_t *class2 = (not_node_class_t *)item1->value;
                    if (not_semantic_idcmp(property1->key, class2->key) == 1)
                    {
                        not_node_basic_t *basic1 = (not_node_basic_t *)property1->key->value;

                        not_error_semantic_by_node(property1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                   basic1->value, class2->key->position.line, class2->key->position.column);
                        return -1;
                    }
                }
                else if (item1->kind == NODE_KIND_FUN)
                {
                    not_node_fun_t *fun1 = (not_node_fun_t *)item1->value;

                    if (not_semantic_idcmp(property1->key, fun1->key) == 1)
                    {
                        not_node_basic_t *basic1 = (not_node_basic_t *)property1->key->value;
                        not_error_semantic_by_node(property1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                   basic1->value, fun1->key->position.line, fun1->key->position.column);
                        return -1;
                    }
                }
                else if (item1->kind == NODE_KIND_PROPERTY)
                {
                    not_node_property_t *property2 = (not_node_property_t *)item1->value;
                    if (not_semantic_idcmp(property1->key, property2->key) == 1)
                    {
                        not_node_basic_t *basic1 = (not_node_basic_t *)property1->key->value;
                        not_error_semantic_by_node(property1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                   basic1->value, property2->key->position.line, property2->key->position.column);
                        return -1;
                    }
                }
            }

            break;
        }
    }

    if (property1->notes != NULL)
    {
        int32_t r1 = not_semantic_notes(property1->notes);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (property1->type != NULL)
    {
        int32_t r1 = not_semantic_expression(property1->type);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (property1->value != NULL)
    {
        int32_t r1 = not_semantic_expression(property1->value);
        if (r1 == -1)
        {
            return -1;
        }
    }

    return 0;
}

static int32_t
not_semantic_heritage(not_node_t *node)
{
    not_node_heritage_t *heritage1 = (not_node_heritage_t *)node->value;

    for (not_node_t *item1 = node->previous; item1 != NULL; item1 = item1->previous)
    {
        assert(item1->kind == NODE_KIND_HERITAGE);

        not_node_heritage_t *heritage2 = (not_node_heritage_t *)item1->value;
        if (not_semantic_idcmp(heritage1->key, heritage2->key) == 1)
        {
            not_node_basic_t *basic1 = (not_node_basic_t *)heritage1->key->value;

            not_error_semantic_by_node(heritage1->key, "'%s' already defined, previous in (%lld:%lld)",
                                       basic1->value, heritage2->key->position.line, heritage2->key->position.column);
            return -1;
        }
    }

    for (not_node_t *node1 = node->parent; node1 != NULL; node1 = node1->parent)
    {
        if (node1->kind == NODE_KIND_CLASS)
        {
            not_node_class_t *class1 = (not_node_class_t *)node1->value;

            if (class1->generics != NULL)
            {
                for (not_node_t *item1 = class1->generics; item1 != NULL; item1 = item1->next)
                {
                    assert(item1->kind == NODE_KIND_GENERIC);

                    not_node_generic_t *generic1 = (not_node_generic_t *)item1->value;
                    if (not_semantic_idcmp(heritage1->key, generic1->key) == 1)
                    {
                        not_node_basic_t *basic1 = (not_node_basic_t *)heritage1->key->value;

                        not_error_semantic_by_node(heritage1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                   basic1->value, generic1->key->position.line, generic1->key->position.column);
                        return -1;
                    }
                }
            }

            break;
        }
    }

    if (heritage1->key != NULL)
    {
        int32_t r1 = not_semantic_expression(heritage1->key);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (heritage1->type != NULL)
    {
        int32_t r1 = not_semantic_expression(heritage1->type);
        if (r1 == -1)
        {
            return -1;
        }
    }

    return 0;
}

static int32_t
not_semantic_heritages(not_node_t *node)
{
    not_node_block_t *block = (not_node_block_t *)node->value;

    for (not_node_t *item = block->items; item != NULL; item = item->next)
    {
        int32_t r1 = not_semantic_heritage(item);
        if (r1 == -1)
        {
            return -1;
        }
    }
    return 0;
}

static int32_t
not_semantic_class(not_node_t *node)
{
    not_node_class_t *class1 = (not_node_class_t *)node->value;

    for (not_node_t *node1 = node->parent, *subnode = node; node1 != NULL; subnode = node1, node1 = node1->parent)
    {
        if (node1->kind == NODE_KIND_MODULE)
        {
            not_node_block_t *module1 = (not_node_block_t *)node1->value;

            for (not_node_t *item1 = module1->items; item1 != NULL; item1 = item1->next)
            {
                if (item1->id == subnode->id)
                {
                    break;
                }

                if (item1->kind == NODE_KIND_USING)
                {
                    not_node_using_t *using1 = (not_node_using_t *)item1->value;
                    not_node_block_t *packages1 = (not_node_block_t *)using1->packages->value;
                    for (not_node_t *item2 = packages1->items; item2 != NULL; item2 = item2->next)
                    {
                        not_node_package_t *package1 = (not_node_package_t *)item2->value;
                        if (not_semantic_idcmp(package1->key, class1->key) == 1)
                        {
                            not_node_basic_t *basic1 = (not_node_basic_t *)class1->key->value;

                            not_error_semantic_by_node(class1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                       basic1->value, package1->key->position.line, package1->key->position.column);
                            return -1;
                        }
                    }
                }
                else if (item1->kind == NODE_KIND_CLASS)
                {
                    not_node_class_t *class2 = (not_node_class_t *)item1->value;
                    if (not_semantic_idcmp(class1->key, class2->key) == 1)
                    {
                        not_node_basic_t *basic1 = (not_node_basic_t *)class1->key->value;

                        not_error_semantic_by_node(class1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                   basic1->value, class2->key->position.line, class2->key->position.column);
                        return -1;
                    }
                }
                else if (item1->kind == NODE_KIND_FOR)
                {
                    not_node_for_t *for1 = (not_node_for_t *)item1->value;
                    if (for1->key != NULL)
                    {
                        if (not_semantic_idcmp(class1->key, for1->key) == 1)
                        {
                            not_node_basic_t *basic1 = (not_node_basic_t *)class1->key->value;

                            not_error_semantic_by_node(class1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                       basic1->value, for1->key->position.line, for1->key->position.column);
                            return -1;
                        }
                    }
                }
                else if (item1->kind == NODE_KIND_VAR)
                {
                    not_node_var_t *var1 = (not_node_var_t *)item1->value;
                    if (var1->key->kind == NODE_KIND_ID)
                    {
                        if (not_semantic_idcmp(class1->key, var1->key) == 1)
                        {
                            not_node_basic_t *basic1 = (not_node_basic_t *)class1->key->value;

                            not_error_semantic_by_node(class1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                       basic1->value, var1->key->position.line, var1->key->position.column);
                            return -1;
                        }
                    }
                    else
                    {
                        for (not_node_t *item1 = var1->key; item1 != NULL; item1 = item1->next)
                        {
                            assert(item1->kind == NODE_KIND_ENTITY);

                            not_node_entity_t *entity1 = (not_node_entity_t *)item1->value;
                            if (not_semantic_idcmp(class1->key, entity1->key) == 1)
                            {
                                not_node_basic_t *basic1 = (not_node_basic_t *)class1->key->value;

                                not_error_semantic_by_node(class1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                           basic1->value, entity1->key->position.line, entity1->key->position.column);
                                return -1;
                            }
                        }
                    }
                }
            }
            break;
        }
        else if (node1->kind == NODE_KIND_CLASS)
        {
            not_node_class_t *class2 = (not_node_class_t *)node1->value;

            if (class2->generics != NULL)
            {
                not_node_block_t *block1 = (not_node_block_t *)class1->generics->value;

                for (not_node_t *item1 = block1->items; item1 != NULL; item1 = item1->next)
                {
                    assert(item1->kind == NODE_KIND_GENERIC);

                    not_node_generic_t *generic1 = (not_node_generic_t *)item1->value;

                    if (not_semantic_idcmp(class1->key, generic1->key) == 1)
                    {
                        not_node_basic_t *basic1 = (not_node_basic_t *)class1->key->value;

                        not_error_semantic_by_node(class1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                   basic1->value, generic1->key->position.line, generic1->key->position.column);
                        return -1;
                    }
                }
            }

            if (class2->heritages != NULL)
            {
                not_node_block_t *block1 = (not_node_block_t *)class1->generics->value;

                for (not_node_t *item1 = block1->items; item1 != NULL; item1 = item1->next)
                {
                    assert(item1->kind == NODE_KIND_HERITAGE);

                    not_node_heritage_t *heritage1 = (not_node_heritage_t *)item1->value;

                    if (not_semantic_idcmp(class1->key, heritage1->key) == 1)
                    {
                        not_node_basic_t *basic1 = (not_node_basic_t *)class1->key->value;

                        not_error_semantic_by_node(class1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                   basic1->value, heritage1->key->position.line, heritage1->key->position.column);
                        return -1;
                    }
                }
            }

            for (not_node_t *item1 = class2->block; item1 != NULL; item1 = item1->next)
            {
                if (item1->id == node->id)
                {
                    break;
                }

                if (item1->kind == NODE_KIND_CLASS)
                {
                    not_node_class_t *class3 = (not_node_class_t *)item1->value;
                    if (not_semantic_idcmp(class1->key, class3->key) == 1)
                    {
                        not_node_basic_t *basic1 = (not_node_basic_t *)class1->key->value;

                        not_error_semantic_by_node(class1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                   basic1->value, class3->key->position.line, class3->key->position.column);
                        return -1;
                    }
                }
                else if (item1->kind == NODE_KIND_FUN)
                {
                    not_node_fun_t *fun1 = (not_node_fun_t *)item1->value;

                    if (not_semantic_idcmp(class1->key, fun1->key) == 1)
                    {
                        not_node_basic_t *basic1 = (not_node_basic_t *)class1->key->value;
                        not_error_semantic_by_node(class1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                   basic1->value, fun1->key->position.line, fun1->key->position.column);
                        return -1;
                    }
                }
                else if (item1->kind == NODE_KIND_PROPERTY)
                {
                    not_node_property_t *property1 = (not_node_property_t *)item1->value;
                    if (not_semantic_idcmp(class1->key, property1->key) == 1)
                    {
                        not_node_basic_t *basic1 = (not_node_basic_t *)class1->key->value;
                        not_error_semantic_by_node(class1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                   basic1->value, property1->key->position.line, property1->key->position.column);
                        return -1;
                    }
                }
            }

            break;
        }
    }

    if (class1->notes != NULL)
    {
        int32_t r1 = not_semantic_notes(class1->notes);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (class1->generics != NULL)
    {
        if ((class1->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
        {
            not_error_semantic_by_node(class1->key, "Generic type in static class",
                                       __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = not_semantic_generics(class1->generics);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (class1->heritages != NULL)
    {
        if ((class1->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
        {
            not_error_semantic_by_node(class1->key, "Generic type in inheritance",
                                       __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = not_semantic_heritages(class1->heritages);
        if (r1 == -1)
        {
            return -1;
        }
    }

    for (not_node_t *item = class1->block; item != NULL; item = item->next)
    {
        if (item->kind == NODE_KIND_CLASS)
        {
            if ((class1->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
            {
                not_node_class_t *class2 = (not_node_class_t *)item->value;
                if ((class2->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                {
                    not_error_semantic_by_node(class2->key, "Static class(%lld:%lld), members must be static",
                                               class1->key->position.line, class1->key->position.column);
                    return -1;
                }
            }

            int32_t r1 = not_semantic_class(item);
            if (r1 == -1)
            {
                return -1;
            }
        }
        else if (item->kind == NODE_KIND_FUN)
        {
            if ((class1->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
            {
                not_node_fun_t *fun1 = (not_node_fun_t *)item->value;
                if ((fun1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                {
                    not_error_semantic_by_node(fun1->key, "Static class(%lld:%lld), members must be static",
                                               class1->key->position.line, class1->key->position.column);
                    return -1;
                }
            }

            int32_t r1 = not_semantic_fun(item);
            if (r1 == -1)
            {
                return -1;
            }
        }
        else if (item->kind == NODE_KIND_PROPERTY)
        {
            if ((class1->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
            {
                not_node_property_t *property1 = (not_node_property_t *)item->value;
                if ((property1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                {
                    not_error_semantic_by_node(property1->key, "Static class(%lld:%lld), members must be static",
                                               class1->key->position.line, class1->key->position.column);
                    return -1;
                }
            }

            not_node_property_t *property1 = (not_node_property_t *)item->value;
            if ((property1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
            {
                int32_t r1 = not_semantic_property(item);
                if (r1 == -1)
                {
                    return -1;
                }
            }
        }
    }

    return 0;
}

static int32_t
not_semantic_package(not_node_t *node)
{
    not_node_package_t *package1 = (not_node_package_t *)node->value;

    for (not_node_t *item1 = node->previous; item1 != NULL; item1 = item1->previous)
    {
        not_node_package_t *package2 = (not_node_package_t *)item1->value;
        if (not_semantic_idcmp(package1->key, package2->key) == 1)
        {
            not_node_basic_t *basic1 = (not_node_basic_t *)package1->key->value;

            not_error_semantic_by_node(package1->key, "'%s' already defined, previous in (%lld:%lld)",
                                       basic1->value, package2->key->position.line, package2->key->position.column);
            return -1;
        }
    }

    for (not_node_t *node1 = node->parent, *subnode = node; node1 != NULL; subnode = node1, node1 = node1->parent)
    {
        if (node1->kind == NODE_KIND_MODULE)
        {
            not_node_block_t *module1 = (not_node_block_t *)node1->value;

            for (not_node_t *item1 = module1->items; item1 != NULL; item1 = item1->next)
            {
                if (item1->id == subnode->id)
                {
                    break;
                }

                if (item1->kind == NODE_KIND_USING)
                {
                    not_node_using_t *using1 = (not_node_using_t *)item1->value;
                    not_node_block_t *packages1 = (not_node_block_t *)using1->packages->value;
                    for (not_node_t *item2 = packages1->items; item2 != NULL; item2 = item2->next)
                    {
                        assert(item2->kind == NODE_KIND_PACKAGE);

                        not_node_package_t *package2 = (not_node_package_t *)item1->value;
                        if (not_semantic_idcmp(package1->key, package2->key) == 1)
                        {
                            not_node_basic_t *basic1 = (not_node_basic_t *)package1->key->value;

                            not_error_semantic_by_node(package1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                       basic1->value, package2->key->position.line, package2->key->position.column);
                            return -1;
                        }
                    }
                }
                else if (item1->kind == NODE_KIND_CLASS)
                {
                    not_node_class_t *class1 = (not_node_class_t *)item1->value;

                    if (not_semantic_idcmp(package1->key, class1->key) == 1)
                    {
                        not_node_basic_t *basic1 = (not_node_basic_t *)package1->key->value;

                        not_error_semantic_by_node(package1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                   basic1->value, class1->key->position.line, class1->key->position.column);
                        return -1;
                    }
                }
                else if (item1->kind == NODE_KIND_FOR)
                {
                    not_node_for_t *for1 = (not_node_for_t *)item1->value;
                    if (for1->key != NULL)
                    {
                        if (not_semantic_idcmp(package1->key, for1->key) == 1)
                        {
                            not_node_basic_t *basic1 = (not_node_basic_t *)package1->key->value;

                            not_error_semantic_by_node(package1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                       basic1->value, for1->key->position.line, for1->key->position.column);
                            return -1;
                        }
                    }
                }
                else if (item1->kind == NODE_KIND_FORIN)
                {
                    not_node_forin_t *for1 = (not_node_forin_t *)item1->value;
                    if (for1->key != NULL)
                    {
                        if (not_semantic_idcmp(package1->key, for1->key) == 1)
                        {
                            not_node_basic_t *basic1 = (not_node_basic_t *)package1->key->value;

                            not_error_semantic_by_node(package1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                       basic1->value, for1->key->position.line, for1->key->position.column);
                            return -1;
                        }
                    }
                }
                else if (item1->kind == NODE_KIND_VAR)
                {
                    not_node_var_t *var1 = (not_node_var_t *)item1->value;
                    if (var1->key->kind == NODE_KIND_ID)
                    {
                        if (not_semantic_idcmp(package1->key, var1->key) == 1)
                        {
                            not_node_basic_t *basic1 = (not_node_basic_t *)package1->key->value;

                            not_error_semantic_by_node(package1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                       basic1->value, var1->key->position.line, var1->key->position.column);
                            return -1;
                        }
                    }
                    else
                    {
                        for (not_node_t *item1 = var1->key; item1 != NULL; item1 = item1->next)
                        {
                            assert(item1->kind == NODE_KIND_ENTITY);

                            not_node_entity_t *entity1 = (not_node_entity_t *)item1->value;

                            if (not_semantic_idcmp(package1->key, entity1->key) == 1)
                            {
                                not_node_basic_t *basic1 = (not_node_basic_t *)package1->key->value;

                                not_error_semantic_by_node(package1->key, "'%s' already defined, previous in (%lld:%lld)",
                                                           basic1->value, entity1->key->position.line, entity1->key->position.column);
                                return -1;
                            }
                        }
                    }
                }
            }

            break;
        }
    }

    if (package1->value != NULL)
    {
        int32_t r1 = not_semantic_expression(package1->value);
        if (r1 == -1)
        {
            return -1;
        }
    }

    return 0;
}

static int32_t
not_semantic_packages(not_node_t *node)
{
    not_node_block_t *block = (not_node_block_t *)node->value;

    for (not_node_t *item = block->items; item != NULL; item = item->next)
    {
        int32_t r1 = not_semantic_package(item);
        if (r1 == -1)
        {
            return -1;
        }
    }

    return 0;
}

static int32_t
not_semantic_using(not_node_t *node)
{
    not_node_using_t *using1 = (not_node_using_t *)node->value;

    if (using1->packages != NULL)
    {
        int32_t r1 = not_semantic_packages(using1->packages);
        if (r1 == -1)
        {
            return -1;
        }
    }

    not_node_t *node1 = using1->path;
    not_node_basic_t *basic1 = (not_node_basic_t *)node1->value;

    not_module_t *module_entry = not_repository_load(basic1->value);
    if (module_entry == ERROR)
    {
        not_error_semantic_by_node(node, "Error:incapable of loading the modulus");
        return -1;
    }

    return 0;
}

int32_t
not_semantic_module(not_node_t *node)
{
    not_node_block_t *module = (not_node_block_t *)node->value;

    for (not_node_t *item = module->items; item != NULL; item = item->next)
    {
        if (item->kind == NODE_KIND_USING)
        {
            int32_t r1 = not_semantic_using(item);
            if (r1 == -1)
            {
                return -1;
            }
        }
        else if (item->kind == NODE_KIND_CLASS)
        {
            int32_t r1 = not_semantic_class(item);
            if (r1 == -1)
            {
                return -1;
            }
        }
        else if (item->kind == NODE_KIND_IF)
        {
            int32_t r1 = not_semantic_if(item);
            if (r1 == -1)
            {
                return -1;
            }
        }
        else if (item->kind == NODE_KIND_FOR)
        {
            int32_t r1 = not_semantic_for(item);
            if (r1 == -1)
            {
                return -1;
            }
        }
        else if (item->kind == NODE_KIND_FORIN)
        {
            int32_t r1 = not_semantic_forin(item);
            if (r1 == -1)
            {
                return -1;
            }
        }
        else if (item->kind == NODE_KIND_TRY)
        {
            int32_t r1 = not_semantic_try(item);
            if (r1 == -1)
            {
                return -1;
            }
        }
        else if (item->kind == NODE_KIND_VAR)
        {
            int32_t r1 = not_semantic_var(item);
            if (r1 == -1)
            {
                return -1;
            }
        }
    }

    return 0;
}
