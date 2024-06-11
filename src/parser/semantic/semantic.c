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
#include "../../module.h"

#include "semantic.h"

int32_t
sy_semantic_idcmp(sy_node_t *n1, sy_node_t *n2)
{
	sy_node_basic_t *nb1 = (sy_node_basic_t *)n1->value;
	sy_node_basic_t *nb2 = (sy_node_basic_t *)n2->value;

	//printf("%s %s\n", nb1->value, nb2->value);

	return (strcmp(nb1->value, nb2->value) == 0);
}

int32_t
sy_semantic_strcmp(sy_node_t *n1, char *name)
{
	sy_node_basic_t *nb1 = (sy_node_basic_t *)n1->value;
	return (strcmp(nb1->value, name) == 0);
}


static int32_t
sy_semantic_var(sy_node_t *node);

static int32_t
sy_semantic_parameter(sy_node_t *node);

static int32_t
sy_semantic_parameters(sy_node_t *node);

static int32_t
sy_semantic_arguments(sy_node_t *node);

static int32_t
sy_semantic_fields(sy_node_t *node);

static int32_t
sy_semantic_generics(sy_node_t *node);

static int32_t
sy_semantic_body(sy_node_t *node);

static int32_t
sy_semantic_expression(sy_node_t *node);

static int32_t
sy_semantic_assign(sy_node_t *node);


static int32_t
sy_semantic_id(sy_node_t *node)
{
    return 0;
}

static int32_t
sy_semantic_number(sy_node_t *node)
{
    return 0;
}

static int32_t
sy_semantic_char(sy_node_t *node)
{
    return 0;
}

static int32_t
sy_semantic_string(sy_node_t *node)
{
    return 0; 
}


static int32_t
sy_semantic_null(sy_node_t *node)
{
    return 0; 
}

static int32_t
sy_semantic_kint(sy_node_t *node)
{
    return 0;
}

static int32_t
sy_semantic_kfloat(sy_node_t *node)
{
    return 0;
}

static int32_t
sy_semantic_kchar(sy_node_t *node)
{
    return 0;
}

static int32_t
sy_semantic_kstring(sy_node_t *node)
{
    return 0;
}


static int32_t
sy_semantic_lambda(sy_node_t *node)
{
    sy_node_lambda_t *fun1 = (sy_node_lambda_t *)node->value;

    if (fun1->generics != NULL)
    {
        int32_t r1 = sy_semantic_generics(fun1->generics);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (fun1->parameters != NULL)
    {
        int32_t r1 = sy_semantic_parameters(fun1->parameters);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (fun1->body != NULL)
    {
        if (fun1->body->kind == NODE_KIND_BODY)
        {
            int32_t r1 = sy_semantic_body(fun1->body);
            if (r1 == -1)
            {
                return -1;
            }
        }
        else
        {
            int32_t r1 = sy_semantic_expression(fun1->body);
            if (r1 == -1)
            {
                return -1;
            }
        }
    }

	return 0;
}

static int32_t
sy_semantic_parenthesis(sy_node_t *node)
{
    sy_node_unary_t *unary1 = (sy_node_unary_t *)node->value;

    int32_t r1 = sy_semantic_expression(unary1->right);
    if (r1 == -1)
    {
        return -1;
    }

    return 0;
}

static int32_t
sy_semantic_primary(sy_node_t *node)
{
    if (node->kind == NODE_KIND_ID)
    {
        return sy_semantic_id(node);
    }
    else

    if (node->kind == NODE_KIND_NUMBER)
    {
        return sy_semantic_number(node);
    }
    else
    if (node->kind == NODE_KIND_CHAR)
    {
        return sy_semantic_char(node);
    }
    else
    if (node->kind == NODE_KIND_STRING)
    {
        return sy_semantic_string(node);
    }
    else

    if (node->kind == NODE_KIND_NULL)
    {
        return sy_semantic_null(node);
    }
    else

    if (node->kind == NODE_KIND_KINT)
    {
        return sy_semantic_kint(node);
    }
    else

    if (node->kind == NODE_KIND_KFLOAT)
    {
        return sy_semantic_kfloat(node);
    }
    else

    if (node->kind == NODE_KIND_KCHAR)
    {
        return sy_semantic_kchar(node);
    }
    else
    if (node->kind == NODE_KIND_KSTRING)
    {
        return sy_semantic_kstring(node);
    }
    else

    if (node->kind == NODE_KIND_TUPLE)
    {
        // return semantic_tuple(node);
        return 0;
    }
    else
    if (node->kind == NODE_KIND_OBJECT)
    {
        // return semantic_object(node);
        return 0;
    }
    else

    if (node->kind == NODE_KIND_LAMBDA)
    {
        return sy_semantic_lambda(node);
    }
    else
    if (node->kind == NODE_KIND_PARENTHESIS)
    {
        return sy_semantic_parenthesis(node);
    }
    
    else
    {
        sy_error_semantic_by_node(node, "not a primary type");
        return -1;
    }
}


static int32_t
sy_semantic_call(sy_node_t *node)
{
    sy_node_carrier_t *carrier1 = (sy_node_carrier_t *)node->value;
    
    int32_t r1 = sy_semantic_expression(carrier1->base);
    if (r1 == -1)
    {
        return -1;
    }

    if (carrier1->data)
    {
        int32_t r2 = sy_semantic_arguments(carrier1->data);
        if (r2 == -1)
        {
            return -1;
        }
    }

    return 0;
}

static int32_t
sy_semantic_array(sy_node_t *node)
{
    sy_node_carrier_t *carrier1 = (sy_node_carrier_t *)node->value;
    
    int32_t r1 = sy_semantic_expression(carrier1->base);
    if (r1 == -1)
    {
        return -1;
    }

    if (carrier1->data != NULL)
    {
        int32_t r1 = sy_semantic_arguments(carrier1->data);
        if (r1 == -1)
        {
            return -1;
        }
        return 0;
    }

    return 0;
}

static int32_t
sy_semantic_attribute(sy_node_t *node)
{
    sy_node_binary_t *binary1 = (sy_node_binary_t *)node->value;

    int32_t r1 = sy_semantic_expression(binary1->left);
    if (r1 == -1)
    {
        return -1;
    }

    int32_t r2 = sy_semantic_expression(binary1->right);
    if (r2 == -1)
    {
        return -1;
    }

    return 0;
}

static int32_t
sy_semantic_pseudonym(sy_node_t *node)
{
    sy_node_carrier_t *carrier1 = (sy_node_carrier_t *)node->value;
    
    int32_t r1 = sy_semantic_expression(carrier1->base);
    if (r1 == -1)
    {
        return -1;
    }

    int32_t r2 = sy_semantic_fields(carrier1->data);
    if (r2 == -1)
    {
        return -1;
    }

    return 0;
}

static int32_t
sy_semantic_postfix(sy_node_t *node)
{
    if (node->kind == NODE_KIND_CALL)
    {
        return sy_semantic_call(node);
    }
    else
    if (node->kind == NODE_KIND_ARRAY)
    {
        return sy_semantic_array(node);
    }
    else
    if (node->kind == NODE_KIND_ATTRIBUTE)
    {
        return sy_semantic_attribute(node);
    }
    else
    if (node->kind == NODE_KIND_PSEUDONYM)
    {
        return sy_semantic_pseudonym(node);
    }
    else
    {
        return sy_semantic_primary(node);
    }
}


static int32_t
sy_semantic_prefix(sy_node_t *node)
{
    if (node->kind == NODE_KIND_TILDE)
    {
        sy_node_unary_t *unary = (sy_node_unary_t *)node->value;
        int32_t r1 = sy_semantic_expression(unary->right);
        if (r1 == -1)
        {
            return -1;
        }
        return 0;
    }
    else
    if (node->kind == NODE_KIND_POS)
    {
        sy_node_unary_t *unary = (sy_node_unary_t *)node->value;
        int32_t r1 = sy_semantic_expression(unary->right);
        if (r1 == -1)
        {
            return -1;
        }
        return 0;
    }
    else
    if (node->kind == NODE_KIND_NEG)
    {
        sy_node_unary_t *unary = (sy_node_unary_t *)node->value;
        int32_t r1 = sy_semantic_expression(unary->right);
        if (r1 == -1)
        {
            return -1;
        }
        return 0;
    }
    else
    if (node->kind == NODE_KIND_NOT)
    {
        sy_node_unary_t *unary = (sy_node_unary_t *)node->value;
        int32_t r1 = sy_semantic_expression(unary->right);
        if (r1 == -1)
        {
            return -1;
        }
        return 0;
    }
    else
    if (node->kind == NODE_KIND_SIZEOF)
    {
        sy_node_unary_t *unary = (sy_node_unary_t *)node->value;
        int32_t r1 = sy_semantic_expression(unary->right);
        if (r1 == -1)
        {
            return -1;
        }
        return 0;
    }
    else
    if (node->kind == NODE_KIND_TYPEOF)
    {
        sy_node_unary_t *unary = (sy_node_unary_t *)node->value;
        int32_t r1 = sy_semantic_expression(unary->right);
        if (r1 == -1)
        {
            return -1;
        }
        return 0;
    }
    else
    {
        return sy_semantic_postfix(node);
    }
}

static int32_t
sy_semantic_pow(sy_node_t *node)
{
    if (node->kind == NODE_KIND_POW)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;

        if (sy_semantic_pow(binary->left) < 0)
        {
            return -1;
        }

        if (sy_semantic_prefix(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else
    {
        return sy_semantic_prefix(node);
    }
}

static int32_t
sy_semantic_multipicative(sy_node_t *node)
{
    if (node->kind == NODE_KIND_MUL)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;

        if (sy_semantic_multipicative(binary->left) < 0)
        {
            return -1;
        }

        if (sy_semantic_pow(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else
    if (node->kind == NODE_KIND_DIV)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;

        if (sy_semantic_multipicative(binary->left) < 0)
        {
            return -1;
        }

        if (sy_semantic_pow(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else
    if (node->kind == NODE_KIND_MOD)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;

        if (sy_semantic_multipicative(binary->left) < 0)
        {
            return -1;
        }

        if (sy_semantic_pow(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else
    if (node->kind == NODE_KIND_EPI)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;

        if (sy_semantic_multipicative(binary->left) < 0)
        {
            return -1;
        }

        if (sy_semantic_pow(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else
    {
        return sy_semantic_pow(node);
    }
}

static int32_t
sy_semantic_addative(sy_node_t *node)
{
    if (node->kind == NODE_KIND_PLUS)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;

        if (sy_semantic_addative(binary->left) < 0)
        {
            return -1;
        }

        if (sy_semantic_multipicative(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else
    if (node->kind == NODE_KIND_MINUS)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;

        if (sy_semantic_addative(binary->left) < 0)
        {
            return -1;
        }

        if (sy_semantic_multipicative(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else
    {
        return sy_semantic_multipicative(node);
    }
}

static int32_t
sy_semantic_shifting(sy_node_t *node)
{
    if (node->kind == NODE_KIND_SHR)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;

        if (sy_semantic_addative(binary->left) < 0)
        {
            return -1;
        }

        if (sy_semantic_addative(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else
    if (node->kind == NODE_KIND_SHL)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;

        if (sy_semantic_addative(binary->left) < 0)
        {
            return -1;
        }

        if (sy_semantic_addative(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else
    {
        return sy_semantic_addative(node);
    }
}

static int32_t
sy_semantic_relational(sy_node_t *node)
{
    if (node->kind == NODE_KIND_LT)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;

        if (sy_semantic_shifting(binary->left) < 0)
        {
            return -1;
        }

        if (sy_semantic_shifting(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else
    if (node->kind == NODE_KIND_LE)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;

        if (sy_semantic_shifting(binary->left) < 0)
        {
            return -1;
        }

        if (sy_semantic_shifting(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else
    if (node->kind == NODE_KIND_GT)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;

        if (sy_semantic_shifting(binary->left) < 0)
        {
            return -1;
        }

        if (sy_semantic_shifting(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else
    if (node->kind == NODE_KIND_GE)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;

        if (sy_semantic_shifting(binary->left) < 0)
        {
            return -1;
        }

        if (sy_semantic_shifting(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else
    {
        return sy_semantic_shifting(node);
    }
}

static int32_t
sy_semantic_equality(sy_node_t *node)
{
    if (node->kind == NODE_KIND_EQ)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;

        if (sy_semantic_relational(binary->left) < 0)
        {
            return -1;
        }

        if (sy_semantic_relational(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else
    if (node->kind == NODE_KIND_NEQ)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;

        if (sy_semantic_relational(binary->left) < 0)
        {
            return -1;
        }

        if (sy_semantic_relational(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else
    {
        return sy_semantic_relational(node);
    }
}

static int32_t
sy_semantic_bitwise_and(sy_node_t *node)
{
    if (node->kind == NODE_KIND_AND)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;

        if (sy_semantic_bitwise_and(binary->left) < 0)
        {
            return -1;
        }

        if (sy_semantic_equality(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else
    {
        return sy_semantic_equality(node);
    }
}

static int32_t
sy_semantic_bitwise_xor(sy_node_t *node)
{
    if (node->kind == NODE_KIND_XOR)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;

        if (sy_semantic_bitwise_xor(binary->left) < 0)
        {
            return -1;
        }

        if (sy_semantic_bitwise_and(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else
    {
        return sy_semantic_bitwise_and(node);
    }
}

static int32_t
sy_semantic_bitwise_or(sy_node_t *node)
{
    if (node->kind == NODE_KIND_OR)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;

        if (sy_semantic_bitwise_or(binary->left) < 0)
        {
            return -1;
        }

        if (sy_semantic_bitwise_xor(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else
    {
        return sy_semantic_bitwise_xor(node);
    }
}

static int32_t
sy_semantic_logical_and(sy_node_t *node)
{
    if (node->kind == NODE_KIND_LAND)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;

        if (sy_semantic_logical_and(binary->left) < 0)
        {
            return -1;
        }

        if (sy_semantic_bitwise_or(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else
    {
        return sy_semantic_bitwise_or(node);
    }
}

static int32_t
sy_semantic_logical_or(sy_node_t *node)
{
    if (node->kind == NODE_KIND_LOR)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        
        if (sy_semantic_logical_or(binary->left) < 0)
        {
            return -1;
        }

        if (sy_semantic_logical_and(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else
    {
        return sy_semantic_logical_and(node);
    }
}

static int32_t
sy_semantic_instanceof(sy_node_t *node)
{
    if (node->kind == NODE_KIND_INSTANCEOF)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        
        if (sy_semantic_logical_or(binary->left) < 0)
        {
            return -1;
        }

        if (sy_semantic_logical_or(binary->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else
    {
        return sy_semantic_logical_or(node);
    }
}

static int32_t
sy_semantic_conditional(sy_node_t *node)
{
    if (node->kind == NODE_KIND_CONDITIONAL)
    {
        sy_node_triple_t *triple = (sy_node_triple_t *)node->value;

        if (sy_semantic_instanceof(triple->base) < 0)
        {
            return -1;
        }

        if (sy_semantic_conditional(triple->left) < 0)
        {
            return -1;
        }

        if (sy_semantic_conditional(triple->right) < 0)
        {
            return -1;
        }

        return 0;
    }
    else
    {
        return sy_semantic_instanceof(node);
    }
}

static int32_t
sy_semantic_expression(sy_node_t *node)
{
	return sy_semantic_conditional(node);
}

static int32_t
sy_semantic_assign(sy_node_t *node)
{
    return 0;
}


static int32_t
sy_semantic_if(sy_node_t *node)
{
    sy_node_if_t *if1 = (sy_node_if_t *)node->value;

    if (if1->condition != NULL)
    {
        int32_t r1 = sy_semantic_expression(if1->condition);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (if1->then_body != NULL)
    {
        int32_t r1 = sy_semantic_body(if1->then_body);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (if1->else_body != NULL)
    {
        sy_node_t *else_body1 = if1->else_body;

        if (else_body1->kind == NODE_KIND_IF)
        {
            int32_t r1 = sy_semantic_if(if1->else_body);
            if (r1 == -1)
            {
                return -1;
            }
        }
        else
        {
            int32_t r1 = sy_semantic_body(if1->else_body);
            if (r1 == -1)
            {
                return -1;
            }
        }
    }

	return 0;
}

static int32_t
sy_semantic_for(sy_node_t *node)
{
    sy_node_for_t *for1 = (sy_node_for_t *)node->value;

    if (for1->key != NULL)
    {
        for (sy_node_t *node1 = node->parent, *subnode = node; node1 != NULL;subnode = node1, node1 = node1->parent)
        {
            if (node1->kind == NODE_KIND_CATCH)
            {
                sy_node_catch_t *catch1 = (sy_node_catch_t *)node1->value;

                for (sy_node_t *item1 = catch1->parameters;item1 != NULL;item1 = item1->next)
                {
                    assert (item1->kind == NODE_KIND_PARAMETER);

                    sy_node_parameter_t *parameter1 = (sy_node_parameter_t *)item1->value;
                    if (sy_semantic_idcmp(for1->key, parameter1->key) == 1)
                    {
                        sy_node_basic_t *basic1 = (sy_node_basic_t *)for1->key->value;
                        sy_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                            basic1->value, parameter1->key->position.line, parameter1->key->position.column);
                        return -1;
                    }
                }
                break;
            }
            else
            if (node1->kind == NODE_KIND_FOR)
            {
                sy_node_for_t *for2 = (sy_node_for_t *)node1->value;

                for (sy_node_t *item1 = for2->initializer;item1 != NULL;item1 = item1->next)
                {
                    if (item1->kind == NODE_KIND_VAR)
                    {
                        sy_node_var_t *var2 = (sy_node_var_t *)item1->value;
                        
                        if (var2->key->kind == NODE_KIND_ID)
                        {
                            if (sy_semantic_idcmp(for1->key, var2->key) == 1)
                            {
                                sy_node_basic_t *basic1 = (sy_node_basic_t *)for1->key->value;

                                sy_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                    basic1->value, var2->key->position.line, var2->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            for (sy_node_t *item2 = var2->key;item2 != NULL;item2 = item2->next)
                            {
                                assert (item2->kind == NODE_KIND_ENTITY);

                                sy_node_entity_t *entity1 = (sy_node_entity_t *)item2->value;
                                if (sy_semantic_idcmp(for1->key, entity1->key) == 1)
                                {
                                    sy_node_basic_t *basic1 = (sy_node_basic_t *)for1->key->value;

                                    sy_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                        basic1->value, entity1->key->position.line, entity1->key->position.column);
                                    return -1;
                                }
                            }
                        }
                    }
                }
                break;
            }
            else
            if (node1->kind == NODE_KIND_BODY)
            {
                sy_node_body_t *body1 = (sy_node_body_t *)node1->value;

                for (sy_node_t *item1 = body1->declaration;item1 != NULL;item1 = item1->next)
                {
                    if (item1->id == subnode->id)
                    {
                        break;
                    }

                    if (item1->kind == NODE_KIND_FOR)
                    {
                        sy_node_for_t *for2 = (sy_node_for_t *)item1->value;
                        if (for2->key != NULL)
                        {
                            if (sy_semantic_idcmp(for1->key, for2->key) == 1)
                            {
                                sy_node_basic_t *basic1 = (sy_node_basic_t *)for1->key->value;

                                sy_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                    basic1->value, for2->key->position.line, for2->key->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item1->kind == NODE_KIND_VAR)
                    {
                        sy_node_var_t *var2 = (sy_node_var_t *)item1->value;
                        if (var2->key->kind == NODE_KIND_ID)
                        {
                            if (sy_semantic_idcmp(for1->key, var2->key) == 1)
                            {
                                sy_node_basic_t *basic1 = (sy_node_basic_t *)for1->key->value;

                                sy_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                    basic1->value, var2->key->position.line, var2->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            sy_node_block_t *block = (sy_node_block_t *)var2->key->value;
                            for (sy_node_t *item2 = block->items;item2 != NULL;item2 = item2->next)
                            {
                                assert (item2->kind == NODE_KIND_ENTITY);
                                sy_node_entity_t *entity1 = (sy_node_entity_t *)item2->value;
                                if (sy_semantic_idcmp(for1->key, entity1->key) == 1)
                                {
                                    sy_node_basic_t *basic1 = (sy_node_basic_t *)for1->key->value;

                                    sy_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                        basic1->value, entity1->key->position.line, entity1->key->position.column);
                                    return -1;
                                }
                            }
                        }
                    }
                }
                
                continue;
            }
            else
            if (node1->kind == NODE_KIND_FUN)
            {
                sy_node_fun_t *fun2 = (sy_node_fun_t *)node1->value;

                if (fun2->generics != NULL)
                {
                    for (sy_node_t *item1 = fun2->generics;item1 != NULL;item1 = item1->next)
                    {
                        assert (item1->kind == NODE_KIND_GENERIC);

                        sy_node_generic_t *generic1 = (sy_node_generic_t *)item1->value;
                        if (sy_semantic_idcmp(for1->key, generic1->key) == 1)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)for1->key->value;

                            sy_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                basic1->value, generic1->key->position.line, generic1->key->position.column);
                            return -1;
                        }
                    }
                }

                if (fun2->parameters != NULL)
                {
                    for (sy_node_t *item2 = fun2->parameters;item2 != NULL;item2 = item2->next)
                    {
                        assert (item2->kind == NODE_KIND_PARAMETER);

                        sy_node_parameter_t *parameter1 = (sy_node_parameter_t *)item2->value;
                        if (sy_semantic_idcmp(for1->key, parameter1->key) == 1)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)for1->key->value;

                            sy_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                basic1->value, parameter1->key->position.line, parameter1->key->position.column);
                            return -1;
                        }
                    }
                }
                break;
            }
            else
            if (node1->kind == NODE_KIND_MODULE)
            {
                sy_node_block_t *module1 = (sy_node_block_t *)node1->value;

                for (sy_node_t *item1 = module1->items;item1 != NULL;item1 = item1->next)
                {
                    if (item1->id == subnode->id)
                    {
                        break;
                    }

                    if (item1->kind == NODE_KIND_USING)
                    {
                        sy_node_using_t *using1 = (sy_node_using_t *)item1->value;
                        sy_node_block_t *packages1 = (sy_node_block_t *)using1->packages->value;
                        for (sy_node_t *item2 = packages1->items;item2 != NULL;item2 = item2->next)
                        {
                            assert (item2->kind == NODE_KIND_PACKAGE);

                            sy_node_package_t *package2 = (sy_node_package_t *)item2->value;
                            if (sy_semantic_idcmp(for1->key, package2->key) == 1)
                            {
                                sy_node_basic_t *basic1 = (sy_node_basic_t *)for1->key->value;

                                sy_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                    basic1->value, package2->key->position.line, package2->key->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item1->kind == NODE_KIND_CLASS)
                    {
                        sy_node_class_t *class1 = (sy_node_class_t *)item1->value;

                        if (sy_semantic_idcmp(for1->key, class1->key) == 1)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)for1->key->value;

                            sy_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                basic1->value, class1->key->position.line, class1->key->position.column);
                            return -1;
                        }
                    }
                    else
                    if (item1->kind == NODE_KIND_FOR)
                    {
                        sy_node_for_t *for2 = (sy_node_for_t *)item1->value;
                        if (for2->key != NULL)
                        {
                            if (sy_semantic_idcmp(for1->key, for2->key) == 1)
                            {
                                sy_node_basic_t *basic1 = (sy_node_basic_t *)for1->key->value;

                                sy_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                    basic1->value, for2->key->position.line, for2->key->position.column);
                                return -1;
                            }
                        }
                    }
                    else
                    if (item1->kind == NODE_KIND_VAR)
                    {
                        sy_node_var_t *var2 = (sy_node_var_t *)item1->value;
                        if (var2->key->kind == NODE_KIND_ID)
                        {
                            if (sy_semantic_idcmp(for1->key, var2->key) == 1)
                            {
                                sy_node_basic_t *basic1 = (sy_node_basic_t *)for1->key->value;

                                sy_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
                                    basic1->value, var2->key->position.line, var2->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            sy_node_block_t *block = (sy_node_block_t *)var2->key->value;
                            for (sy_node_t *item2 = block->items;item2 != NULL;item2 = item2->next)
                            {
                                assert (item2->kind == NODE_KIND_ENTITY);
                                sy_node_entity_t *entity1 = (sy_node_entity_t *)item2->value;
                                if (sy_semantic_idcmp(for1->key, entity1->key) == 1)
                                {
                                    sy_node_basic_t *basic1 = (sy_node_basic_t *)for1->key->value;

                                    sy_error_semantic_by_node(for1->key, "'%s' already defined, previous in (%lld:%lld)",
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
        for (sy_node_t *item1 = for1->initializer;item1 != NULL;item1 = item1->next)
        {
            if (item1->kind == NODE_KIND_VAR)
            {
                int32_t r1 = sy_semantic_var(item1);
                if (r1 == -1)
                {
                    return -1;
                }
            }
            else
            {
                int32_t r1 = sy_semantic_assign(item1);
                if (r1 == -1)
                {
                    return -1;
                }
            }
        }
    }

    if (for1->condition != NULL)
    {
        int32_t r1 = sy_semantic_expression(for1->condition);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (for1->body != NULL)
    {
        int32_t r2 = sy_semantic_body(for1->body);
        if (r2 == -1)
        {
            return -1;
        }
    }

    if (for1->incrementor != NULL)
    {
        for (sy_node_t *item1 = for1->incrementor;item1 != NULL;item1 = item1->next)
        {
            int32_t r2 = sy_semantic_assign(item1);
            if (r2 == -1)
            {
                return -1;
            }
        }
    }

	return 0;
}

static int32_t
sy_semantic_entity(sy_node_t *node)
{
    sy_node_entity_t *entity1 = (sy_node_entity_t *)node->value;

    for (sy_node_t *item1 = node->previous; item1 != NULL; item1 = item1->previous)
    {
        assert(item1->kind == NODE_KIND_ENTITY);

        sy_node_entity_t *entity2 = (sy_node_entity_t *)item1->value;
        if (sy_semantic_idcmp(entity1->key, entity2->key) == 1)
        {
            sy_node_basic_t *basic1 = (sy_node_basic_t *)entity1->key->value;

            sy_error_semantic_by_node(entity1->key, "'%s' already defined, previous in (%lld:%lld)",
                basic1->value, entity2->key->position.line, entity2->key->position.column);
            return -1;
        }
    }

    return 0;
}

static int32_t
sy_semantic_set(sy_node_t *node)
{
    sy_node_block_t *block = (sy_node_block_t *)node->value;

    for (sy_node_t *item = block->items;item != NULL;item = item->next)
    {
        int32_t r1 = sy_semantic_entity(item);
        if (r1 == -1)
        {
            return -1;
        }
    }
    return 0;
}

static int32_t
sy_semantic_var(sy_node_t *node)
{
    sy_node_var_t *var1 = (sy_node_var_t *)node->value;

    for (sy_node_t *node1 = node->parent, *subnode = node; node1 != NULL;subnode = node1, node1 = node1->parent)
    {
        if (node1->kind == NODE_KIND_CATCH)
        {
            sy_node_catch_t *catch1 = (sy_node_catch_t *)node1->value;

            for (sy_node_t *item1 = catch1->parameters;item1 != NULL;item1 = item1->next)
            {
                sy_node_parameter_t *parameter2 = (sy_node_parameter_t *)item1->value;
                if (var1->key->kind == NODE_KIND_ID)
                {
                    if (sy_semantic_idcmp(var1->key, parameter2->key) == 1)
                    {
                        sy_node_basic_t *basic1 = (sy_node_basic_t *)var1->key->value;

                        sy_error_semantic_by_node(var1->key, "'%s' already defined, previous in (%lld:%lld)",
                            basic1->value, parameter2->key->position.line, parameter2->key->position.column);
                        return -1;
                    }
                }
                else
                {
                    sy_node_block_t *block = (sy_node_block_t *)var1->key->value;
                    for (sy_node_t *item2 = block->items;item2 != NULL;item2 = item2->next)
                    {
                        sy_node_entity_t *entity1 = (sy_node_entity_t *)item2->value;
                        if (sy_semantic_idcmp(entity1->key, parameter2->key) == 1)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)entity1->key->value;

                            sy_error_semantic_by_node(entity1->key, "'%s' already defined, previous in (%lld:%lld)",
                                basic1->value, parameter2->key->position.line, parameter2->key->position.column);
                            return -1;
                        }
                    }
                }
            }
            
            break;
        }
        else
        if (node1->kind == NODE_KIND_FOR)
        {
            sy_node_for_t *for1 = (sy_node_for_t *)node1->value;

            for (sy_node_t *item1 = for1->initializer;item1 != NULL;item1 = item1->next)
            {
                if (item1 == node)
                {
                    break;
                }

                if (item1->kind == NODE_KIND_VAR)
                {
                    sy_node_var_t *var2 = (sy_node_var_t *)item1->value;
                    if (var2->key->kind == NODE_KIND_ID)
                    {
                        if (var1->key->kind == NODE_KIND_ID)
                        {
                            if (sy_semantic_idcmp(var1->key, var2->key) == 1)
                            {
                                sy_node_basic_t *basic1 = (sy_node_basic_t *)var1->key->value;

                                sy_error_semantic_by_node(var1->key, "'%s' already defined, previous in (%lld:%lld)",
                                    basic1->value, var2->key->position.line, var2->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            for (sy_node_t *item2 = var1->key;item2 != NULL;item2 = item2->next)
                            {
                                assert (item2->kind == NODE_KIND_ENTITY);

                                sy_node_entity_t *entity1 = (sy_node_entity_t *)item2->value;
                                if (sy_semantic_idcmp(entity1->key, var2->key) == 1)
                                {
                                    sy_node_basic_t *basic1 = (sy_node_basic_t *)entity1->key->value;

                                    sy_error_semantic_by_node(entity1->key, "'%s' already defined, previous in (%lld:%lld)",
                                        basic1->value, var2->key->position.line, var2->key->position.column);
                                    return -1;
                                }
                            }
                        }
                    }
                    else
                    {
                        sy_node_block_t *block = (sy_node_block_t *)var1->key->value;
                        for (sy_node_t *item2 = block->items;item2 != NULL;item2 = item2->next)
                        {
                            assert (item2->kind == NODE_KIND_ENTITY);

                            sy_node_entity_t *entity2 = (sy_node_entity_t *)item2->value;
                            if (var1->key->kind == NODE_KIND_ID)
                            {
                                if (sy_semantic_idcmp(var1->key, entity2->key) == 1)
                                {
                                    sy_node_basic_t *basic1 = (sy_node_basic_t *)var1->key->value;

                                    sy_error_semantic_by_node(var1->key, "'%s' already defined, previous in (%lld:%lld)",
                                        basic1->value, entity2->key->position.path, entity2->key->position.line, entity2->key->position.column);
                                    return -1;
                                }
                            }
                            else
                            {
                                for (sy_node_t *item3 = var1->key;item3 != NULL;item3 = item3->next)
                                {
                                    assert (item3->kind == NODE_KIND_ENTITY);

                                    sy_node_entity_t *entity1 = (sy_node_entity_t *)item3->value;
                                    if (sy_semantic_idcmp(entity1->key, entity2->key) == 1)
                                    {
                                        sy_node_basic_t *basic1 = (sy_node_basic_t *)entity1->key->value;

                                        sy_error_semantic_by_node(entity1->key, "'%s' already defined, previous in (%lld:%lld)",
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
        else
        if (node1->kind == NODE_KIND_BODY)
        {
            sy_node_body_t *body1 = (sy_node_body_t *)node1->value;

            for (sy_node_t *item1 = body1->declaration;item1 != NULL;item1 = item1->next)
            {
                if (item1->id == subnode->id)
                {
                    break;
                }

                if (item1->kind == NODE_KIND_FOR)
                {
                    sy_node_for_t *for2 = (sy_node_for_t *)item1->value;
                    if (for2->key != NULL)
                    {
                        if (var1->key->kind == NODE_KIND_ID)
                        {
                            if (sy_semantic_idcmp(var1->key, for2->key) == 1)
                            {
                                sy_node_basic_t *basic1 = (sy_node_basic_t *)var1->key->value;

                                sy_error_semantic_by_node(var1->key, "'%s' already defined, previous in (%lld:%lld)",
                                    basic1->value, for2->key->position.line, for2->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            sy_node_block_t *block = (sy_node_block_t *)var1->key->value;
                            for (sy_node_t *item2 = block->items;item2 != NULL;item2 = item2->next)
                            {
                                assert (item2->kind == NODE_KIND_ENTITY);

                                sy_node_entity_t *entity1 = (sy_node_entity_t *)item2->value;
                                if (sy_semantic_idcmp(entity1->key, for2->key) == 1)
                                {
                                    sy_node_basic_t *basic1 = (sy_node_basic_t *)entity1->key->value;

                                    sy_error_semantic_by_node(entity1->key, "'%s' already defined, previous in (%lld:%lld)",
                                        basic1->value, for2->key->position.line, for2->key->position.column);
                                    return -1;
                                }
                            }
                        }
                    }
                }
                else
                if (item1->kind == NODE_KIND_VAR)
                {
                    sy_node_var_t *var2 = (sy_node_var_t *)item1->value;
                    if (var2->key->kind == NODE_KIND_ID)
                    {
                        if (var1->key->kind == NODE_KIND_ID)
                        {
                            if (sy_semantic_idcmp(var1->key, var2->key) == 1)
                            {
                                sy_node_basic_t *basic1 = (sy_node_basic_t *)var1->key->value;

                                sy_error_semantic_by_node(var1->key, "'%s' already defined, previous in (%lld:%lld)",
                                    basic1->value, var2->key->position.line, var2->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            sy_node_block_t *block = (sy_node_block_t *)var1->key->value;
                            for (sy_node_t *item2 = block->items;item2 != NULL;item2 = item2->next)
                            {
                                assert (item2->kind == NODE_KIND_ENTITY);

                                sy_node_entity_t *entity1 = (sy_node_entity_t *)item2->value;
                                if (sy_semantic_idcmp(entity1->key, var2->key) == 1)
                                {
                                    sy_node_basic_t *basic1 = (sy_node_basic_t *)entity1->key->value;

                                    sy_error_semantic_by_node(entity1->key, "'%s' already defined, previous in (%lld:%lld)",
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
                            sy_node_block_t *block = (sy_node_block_t *)var2->key->value;
                            for (sy_node_t *item2 = block->items;item2 != NULL;item2 = item2->next)
                            {
                                assert (item2->kind == NODE_KIND_ENTITY);

                                sy_node_entity_t *entity1 = (sy_node_entity_t *)item2->value;
                                if (sy_semantic_idcmp(var1->key, entity1->key) == 1)
                                {
                                    sy_node_basic_t *basic1 = (sy_node_basic_t *)var1->key->value;

                                    sy_error_semantic_by_node(var1->key, "'%s' already defined, previous in (%lld:%lld)",
                                        basic1->value, entity1->key->position.line, entity1->key->position.column);
                                    return -1;
                                }
                            }
                        }
                        else
                        {
                            sy_node_block_t *block1 = (sy_node_block_t *)var1->key->value;
                            for (sy_node_t *item2 = block1->items;item2 != NULL;item2 = item2->next)
                            {
                                sy_node_entity_t *entity1 = (sy_node_entity_t *)item2->value;
                                
                                sy_node_block_t *block2 = (sy_node_block_t *)var2->key->value;
                                for (sy_node_t *item3 = block2->items;item3 != NULL;item3 = item3->next)
                                {
                                    assert (item3->kind == NODE_KIND_ENTITY);

                                    sy_node_entity_t *entity2 = (sy_node_entity_t *)item3->value;
                                    if (sy_semantic_idcmp(entity1->key, entity2->key) == 1)
                                    {
                                        sy_node_basic_t *basic1 = (sy_node_basic_t *)entity1->key->value;

                                        sy_error_semantic_by_node(entity1->key, "'%s' already defined, previous in (%lld:%lld)",
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
        else
        if (node1->kind == NODE_KIND_FUN)
        {
            sy_node_fun_t *fun1 = (sy_node_fun_t *)node1->value;

            if (fun1->generics != NULL)
            {
                for (sy_node_t *item1 = fun1->generics;item1 != NULL;item1 = item1->next)
                {
                    sy_node_generic_t *generic2 = (sy_node_generic_t *)item1->value;
                    if (var1->key->kind == NODE_KIND_ID)
                    {
                        if (sy_semantic_idcmp(var1->key, generic2->key) == 1)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)var1->key->value;

                            sy_error_semantic_by_node(var1->key, "'%s' already defined, previous in (%lld:%lld)",
                                basic1->value, generic2->key->position.line, generic2->key->position.column);
                            return -1;
                        }
                    }
                    else
                    {
                        sy_node_block_t *block = (sy_node_block_t *)var1->key->value;
                        for (sy_node_t *item3 = block->items;item3 != NULL;item3 = item3->next)
                        {
                            sy_node_entity_t *entity1 = (sy_node_entity_t *)item3->value;
                            if (sy_semantic_idcmp(entity1->key, generic2->key) == 1)
                            {
                                sy_node_basic_t *basic1 = (sy_node_basic_t *)entity1->key->value;

                                sy_error_semantic_by_node(entity1->key, "'%s' already defined, previous in (%lld:%lld)",
                                    basic1->value, generic2->key->position.line, generic2->key->position.column);
                                return -1;
                            }
                        }
                    }
                }
            }

            if (fun1->parameters != NULL)
            {
                for (sy_node_t *item1 = fun1->parameters;item1 != NULL;item1 = item1->next)
                {
                    sy_node_parameter_t *parameter1 = (sy_node_parameter_t *)item1->value;
                    if (var1->key->kind == NODE_KIND_ID)
                    {
                        if (sy_semantic_idcmp(var1->key, parameter1->key) == 1)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)var1->key->value;

                            sy_error_semantic_by_node(var1->key, "'%s' already defined, previous in (%lld:%lld)",
                                basic1->value, parameter1->key->position.line, parameter1->key->position.column);
                            return -1;
                        }
                    }
                    else
                    {
                        sy_node_block_t *block = (sy_node_block_t *)var1->key->value;
                        for (sy_node_t *item2 = block->items;item2 != NULL;item2 = item2->next)
                        {
                            assert (item2->kind == NODE_KIND_ENTITY);

                            sy_node_entity_t *entity1 = (sy_node_entity_t *)item2->value;
                            if (sy_semantic_idcmp(entity1->key, parameter1->key) == 1)
                            {
                                sy_node_basic_t *basic1 = (sy_node_basic_t *)entity1->key->value;

                                sy_error_semantic_by_node(entity1->key, "'%s' already defined, previous in (%lld:%lld)",
                                    basic1->value, parameter1->key->position.line, parameter1->key->position.column);
                                return -1;
                            }
                        }
                    }
                }
            }

            break;
        }
        else
        if (node1->kind == NODE_KIND_MODULE)
        {
            sy_node_block_t *module1 = (sy_node_block_t *)node1->value;
            for (sy_node_t *item1 = module1->items;item1 != NULL;item1 = item1->next)
            {
                if (item1->id == subnode->id)
                {
                    break;
                }

                if (item1->kind == NODE_KIND_USING)
                {
                    sy_node_using_t *using1 = (sy_node_using_t *)item1->value;
                    sy_node_block_t *packages1 = (sy_node_block_t *)using1->packages->value;
                    for (sy_node_t *item2 = packages1->items;item2 != NULL;item2 = item2->next)
                    {
                        sy_node_package_t *package1 = (sy_node_package_t *)item2->value;
                        
                        if (var1->key->kind == NODE_KIND_ID)
                        {
                            if (sy_semantic_idcmp(var1->key, package1->key) == 1)
                            {
                                sy_node_basic_t *basic1 = (sy_node_basic_t *)var1->key->value;

                                sy_error_semantic_by_node(var1->key, "'%s' already defined, previous in (%lld:%lld)",
                                    basic1->value, package1->key->position.line, package1->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            sy_node_block_t *block = (sy_node_block_t *)var1->key->value;
                            for (sy_node_t *item3 = block->items;item3 != NULL;item3 = item3->next)
                            {
                                sy_node_entity_t *entity1 = (sy_node_entity_t *)item3->value;

                                if (sy_semantic_idcmp(entity1->key, package1->key) == 1)
                                {
                                    sy_node_basic_t *basic1 = (sy_node_basic_t *)entity1->key->value;

                                    sy_error_semantic_by_node(entity1->key, "'%s' already defined, previous in (%lld:%lld)",
                                        basic1->value, package1->key->position.line, package1->key->position.column);
                                    return -1;
                                }
                            }
                        }
                    }
                }
                else
                if (item1->kind == NODE_KIND_CLASS)
                {
                    sy_node_class_t *class1 = (sy_node_class_t *)item1->value;
                    if (var1->key->kind == NODE_KIND_ID)
                    {
                        if (sy_semantic_idcmp(var1->key, class1->key) == 1)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)var1->key->value;

                            sy_error_semantic_by_node(var1->key, "'%s' already defined, previous in (%lld:%lld)",
                                basic1->value, class1->key->position.line, class1->key->position.column);
                            return -1;
                        }
                    }
                    else
                    {
                        sy_node_block_t *block = (sy_node_block_t *)var1->key->value;
                        for (sy_node_t *item2 = block->items;item2 != NULL;item2 = item2->next)
                        {
                            assert (item2->kind == NODE_KIND_ENTITY);

                            sy_node_entity_t *entity1 = (sy_node_entity_t *)item2->value;
                            if (sy_semantic_idcmp(entity1->key, class1->key) == 1)
                            {
                                sy_node_basic_t *basic1 = (sy_node_basic_t *)entity1->key->value;

                                sy_error_semantic_by_node(entity1->key, "'%s' already defined, previous in (%lld:%lld)",
                                    basic1->value, class1->key->position.line, class1->key->position.column);
                                return -1;
                            }
                        }
                    }
                }
                else
                if (item1->kind == NODE_KIND_FOR)
                {
                    sy_node_for_t *for2 = (sy_node_for_t *)item1->value;
                    if (for2->key != NULL)
                    {
                        if (var1->key->kind == NODE_KIND_ID)
                        {
                            if (sy_semantic_idcmp(var1->key, for2->key) == 1)
                            {
                                sy_node_basic_t *basic1 = (sy_node_basic_t *)var1->key->value;

                                sy_error_semantic_by_node(var1->key, "'%s' already defined, previous in (%lld:%lld)",
                                    basic1->value, for2->key->position.line, for2->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            sy_node_block_t *block = (sy_node_block_t *)var1->key->value;
                            for (sy_node_t *item2 = block->items;item2 != NULL;item2 = item2->next)
                            {
                                assert (item2->kind == NODE_KIND_ENTITY);

                                sy_node_entity_t *entity1 = (sy_node_entity_t *)item2->value;
                                if (sy_semantic_idcmp(entity1->key, for2->key) == 1)
                                {
                                    sy_node_basic_t *basic1 = (sy_node_basic_t *)entity1->key->value;

                                    sy_error_semantic_by_node(entity1->key, "'%s' already defined, previous in (%lld:%lld)",
                                        basic1->value, for2->key->position.line, for2->key->position.column);
                                    return -1;
                                }
                            }
                        }
                    }
                }
                else
                if (item1->kind == NODE_KIND_VAR)
                {
                    sy_node_var_t *var2 = (sy_node_var_t *)item1->value;
                    if (var2->key->kind == NODE_KIND_ID)
                    {
                        if (var1->key->kind == NODE_KIND_ID)
                        {
                            if (sy_semantic_idcmp(var1->key, var2->key) == 1)
                            {
                                sy_node_basic_t *basic1 = (sy_node_basic_t *)var1->key->value;

                                sy_error_semantic_by_node(var1->key, "'%s' already defined, previous in (%lld:%lld)",
                                    basic1->value, var2->key->position.line, var2->key->position.column);
                                return -1;
                            }
                        }
                        else
                        {
                            sy_node_block_t *block = (sy_node_block_t *)var1->key->value;
                            for (sy_node_t *item2 = block->items;item2 != NULL;item2 = item2->next)
                            {
                                assert (item2->kind == NODE_KIND_ENTITY);

                                sy_node_entity_t *entity1 = (sy_node_entity_t *)item2->value;
                                if (sy_semantic_idcmp(entity1->key, var2->key) == 1)
                                {
                                    sy_node_basic_t *basic1 = (sy_node_basic_t *)entity1->key->value;

                                    sy_error_semantic_by_node(entity1->key, "'%s' already defined, previous in (%lld:%lld)",
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
                            sy_node_block_t *block = (sy_node_block_t *)var2->key->value;
                            for (sy_node_t *item2 = block->items;item2 != NULL;item2 = item2->next)
                            {
                                assert (item2->kind == NODE_KIND_ENTITY);

                                sy_node_entity_t *entity1 = (sy_node_entity_t *)item2->value;
                                if (sy_semantic_idcmp(var1->key, entity1->key) == 1)
                                {
                                    sy_node_basic_t *basic1 = (sy_node_basic_t *)var1->key->value;

                                    sy_error_semantic_by_node(var1->key, "'%s' already defined, previous in (%lld:%lld)",
                                        basic1->value, entity1->key->position.line, entity1->key->position.column);
                                    return -1;
                                }
                            }
                        }
                        else
                        {
                            sy_node_block_t *block1 = (sy_node_block_t *)var1->key->value;
                            for (sy_node_t *item2 = block1->items;item2 != NULL;item2 = item2->next)
                            {
                                sy_node_entity_t *entity1 = (sy_node_entity_t *)item2->value;
                                sy_node_block_t *block2 = (sy_node_block_t *)var2->key->value;
                                for (sy_node_t *item3 = block2->items;item3 != NULL;item3 = item3->next)
                                {
                                    sy_node_entity_t *entity2 = (sy_node_entity_t *)item3->value;
                                    if (sy_semantic_idcmp(entity1->key, entity2->key) == 1)
                                    {
                                        sy_node_basic_t *basic1 = (sy_node_basic_t *)entity1->key->value;

                                        sy_error_semantic_by_node(entity1->key, "'%s' already defined, previous in (%lld:%lld)",
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
        int32_t r1 = sy_semantic_set(var1->key);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (var1->type != NULL)
    {
        int32_t r1 = sy_semantic_expression(var1->type);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (var1->value != NULL)
    {
        int32_t r1 = sy_semantic_expression(var1->value);
        if (r1 == -1)
        {
            return -1;
        }
    }

	return 0;
}


static int32_t
sy_semantic_field(sy_node_t *node)
{
    sy_node_field_t *field1 = (sy_node_field_t *)node->value;
    
    if (field1->value != NULL)
    {
        if (field1->key->kind != NODE_KIND_ID)
        {
            sy_error_semantic_by_node(field1->key, "'%s' not a name", "Key");
            return -1;
        }
    }

    for (sy_node_t *item1 = node->previous; item1 != NULL; item1 = item1->previous)
    {
        if (field1->value != NULL)
        {
            sy_node_field_t *field2 = (sy_node_field_t *)item1->value;
            if (field2->value != NULL)
            {
                if (sy_semantic_idcmp(field1->key, field2->key) == 1)
                {
                    sy_node_basic_t *key_string1 = field1->key->value;
                    sy_error_semantic_by_node(field1->key, "'%s' already defined, previous in (%lld:%lld)",
                        key_string1->value, field2->key->position.line, field2->key->position.column);
                    return -1;
                }
            }
        }
        else
        {
            sy_node_field_t *field2 = (sy_node_field_t *)item1->value;
            if (field2->value != NULL)
            {
                sy_error_semantic_by_node(field1->key, "the single field is used after the pair field");
                return -1;
            }
        }
    }

    if (field1->key != NULL)
    {
        int32_t r1 = sy_semantic_expression(field1->key);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (field1->value != NULL)
    {
        int32_t r1 = sy_semantic_expression(field1->value);
        if (r1 == -1)
        {
            return -1;
        }
    }

    return 0;
}

static int32_t
sy_semantic_fields(sy_node_t *node)
{
    sy_node_block_t *block = (sy_node_block_t *)node->value;

	for (sy_node_t *item = block->items;item != NULL;item = item->next)
    {
        int32_t r1 = sy_semantic_field(item);
        if (r1 == -1)
        {
            return -1;
        }
    }
    return 0;
}

static int32_t
sy_semantic_argument(sy_node_t *node)
{
    sy_node_argument_t *argument1 = (sy_node_argument_t *)node->value;

    if (argument1->value != NULL)
    {
        if (argument1->key->kind != NODE_KIND_ID)
        {
            sy_error_semantic_by_node(argument1->key, "'%s' not a name",
                "Key");
            return -1;
        }
    }
    
    for (sy_node_t *item1 = node->previous; item1 != NULL; item1 = item1->previous)
    {
        sy_node_argument_t *argument2 = (sy_node_argument_t *)item1->value;

        if ((argument1->value != NULL) && (argument2->value != NULL))
        {
            if (sy_semantic_idcmp(argument1->key, argument2->key) == 1)
            {
                sy_node_basic_t *basic1 = (sy_node_basic_t *)argument1->key->value;

                sy_error_semantic_by_node(argument1->key, "'%s' already defined, previous in (%lld:%lld)",
                    basic1->value, argument2->key->position.line, argument2->key->position.column);
                return -1;
            }
        }
    }

    if (argument1->key != NULL)
    {
        int32_t r1 = sy_semantic_expression(argument1->key);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (argument1->value != NULL)
    {
        int32_t r1 = sy_semantic_expression(argument1->value);
        if (r1 == -1)
        {
            return -1;
        }
    }

    return 0;
}

static int32_t
sy_semantic_arguments(sy_node_t *node)
{
    sy_node_block_t *block = (sy_node_block_t *)node->value;
	for (sy_node_t *item = block->items;item != NULL;item = item->next)
    {
        int32_t r1 = sy_semantic_argument(item);
        if (r1 == -1)
        {
            return -1;
        }
    }
    return 0;
}

static int32_t
sy_semantic_parameter(sy_node_t *node)
{
    sy_node_parameter_t *parameter1 = (sy_node_parameter_t *)node->value;

    for (sy_node_t *item1 = node->previous; item1 != NULL; item1 = item1->previous)
    {
        assert(item1->kind == NODE_KIND_GENERIC);

        sy_node_parameter_t *parameter2 = (sy_node_parameter_t *)item1->value;
        if (sy_semantic_idcmp(parameter1->key, parameter2->key) == 1)
        {
            sy_node_basic_t *basic1 = (sy_node_basic_t *)parameter1->key->value;

            sy_error_semantic_by_node(parameter1->key, "'%s' already defined, previous in (%lld:%lld)",
                basic1->value, parameter2->key->position.line, parameter2->key->position.column);
            return -1;
        }
    }

    for (sy_node_t *node1 = node->parent; node1 != NULL;node1 = node1->parent)
    {
        if (node1->kind == NODE_KIND_FUN)
        {
            sy_node_fun_t *fun2 = (sy_node_fun_t *)node1->value;

            if (fun2->generics != NULL)
            {
                sy_node_block_t *block1 = (sy_node_block_t *)fun2->generics->value;

                for (sy_node_t *item1 = block1->items;item1 != NULL;item1 = item1->next)
                {
                    assert (item1->kind == NODE_KIND_GENERIC);

                    sy_node_generic_t *generic1 = (sy_node_generic_t *)item1->value;
                    if (sy_semantic_idcmp(parameter1->key, generic1->key) == 1)
                    {
                        sy_node_basic_t *basic1 = (sy_node_basic_t *)parameter1->key->value;
                        sy_error_semantic_by_node(parameter1->key, "'%s' already defined, previous in (%lld:%lld)",
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
        int32_t r1 = sy_semantic_expression(parameter1->type);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (parameter1->value != NULL)
    {
        int32_t r1 = sy_semantic_expression(parameter1->value);
        if (r1 == -1)
        {
            return -1;
        }
    }

    return 0;
}

static int32_t
sy_semantic_parameters(sy_node_t *node)
{
    sy_node_block_t *block = (sy_node_block_t *)node->value;

	for (sy_node_t *item = block->items;item != NULL;item = item->next)
    {
        int32_t r1 = sy_semantic_parameter(item);
        if (r1 == -1)
        {
            return -1;
        }
    }
    return 0;
}

static int32_t
sy_semantic_generic(sy_node_t *node)
{
    sy_node_generic_t *generic1 = (sy_node_generic_t *)node->value;
    
    for (sy_node_t *item1 = node->previous; item1 != NULL; item1 = item1->previous)
    {
        assert(item1->kind == NODE_KIND_GENERIC);

        sy_node_generic_t *generic2 = (sy_node_generic_t *)item1->value;
        if (sy_semantic_idcmp(generic1->key, generic2->key) == 1)
        {
            sy_node_basic_t *basic1 = (sy_node_basic_t *)generic1->key->value;

            sy_error_semantic_by_node(generic1->key, "'%s' already defined, previous in (%lld:%lld)",
                basic1->value, generic2->key->position.line, generic2->key->position.column);
            return -1;
        }
    }

    if (generic1->key != NULL)
    {
        int32_t r1 = sy_semantic_expression(generic1->key);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (generic1->type != NULL)
    {
        int32_t r1 = sy_semantic_expression(generic1->type);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (generic1->value != NULL)
    {
        int32_t r1 = sy_semantic_expression(generic1->value);
        if (r1 == -1)
        {
            return -1;
        }
    }

    return 0;
}

static int32_t
sy_semantic_generics(sy_node_t *node)
{
    sy_node_block_t *block = (sy_node_block_t *)node->value;

	for (sy_node_t *item = block->items;item != NULL;item = item->next)
    {
        int32_t r1 = sy_semantic_generic(item);
        if (r1 == -1)
        {
            return -1;
        }
    }
    return 0;
}

static int32_t
sy_semantic_catch(sy_node_t *node)
{
    sy_node_catch_t *catch1 = (sy_node_catch_t *)node->value;

    int32_t r1 = sy_semantic_parameters(catch1->parameters);
    if (r1 == -1)
    {
        return -1;
    } 

    if (catch1->body != NULL)
    {
        int32_t r1 = sy_semantic_body(catch1->body);
        if (r1 == -1)
        {
            return -1;
        }
    }

    return 0;
}

static int32_t
sy_semantic_try(sy_node_t *node)
{
	sy_node_try_t *try1 = (sy_node_try_t *)node->value;

    if (try1->body != NULL)
    {
        int32_t r1 = sy_semantic_body(try1->body);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (try1->catchs != NULL)
    {
        int32_t r1 = sy_semantic_catch(try1->catchs);
        if (r1 == -1)
        {
            return -1;
        }
    }

	return 0;
}

static int32_t
sy_semantic_throw(sy_node_t *node)
{
    return 0;
}

static int32_t
sy_semantic_return(sy_node_t *node)
{
    return 0;
}

static int32_t
sy_semantic_continue(sy_node_t *node)
{
    return 0;
}

static int32_t
sy_semantic_break(sy_node_t *node)
{
    return 0;
}

static int32_t
sy_semantic_statement(sy_node_t *node)
{
    if (node->kind == NODE_KIND_IF)
    {
        int32_t result = sy_semantic_if(node);
        if (result == -1)
        {
            return -1;
        }
    }
    else 
    if (node->kind == NODE_KIND_FOR)
    {
        int32_t result = sy_semantic_for(node);
        if (result == -1)
        {
            return -1;
        }
    }
    else
    if (node->kind == NODE_KIND_TRY)
    {
        int32_t result = sy_semantic_try(node);
        if (result == -1)
        {
            return -1;
        }
    }
    else
    if (node->kind == NODE_KIND_VAR)
    {
        int32_t result = sy_semantic_var(node);
        if (result == -1)
        {
            return -1;
        }
    }
    else
    if (node->kind == NODE_KIND_THROW)
    {
        int32_t result = sy_semantic_throw(node);
        if (result == -1)
        {
            return -1;
        }
    }
    else
    if (node->kind == NODE_KIND_RETURN)
    {
        int32_t result = sy_semantic_return(node);
        if (result == -1)
        {
            return -1;
        }
    }
    else
    if (node->kind == NODE_KIND_CONTINUE)
    {
        int32_t result = sy_semantic_continue(node);
        if (result == -1)
        {
            return -1;
        }
    }
    else
    if (node->kind == NODE_KIND_BREAK)
    {
        int32_t result = sy_semantic_break(node);
        if (result == -1)
        {
            return -1;
        }
    }
    else
    {
        return sy_semantic_assign(node);
    }
    return 0;
}

static int32_t
sy_semantic_body(sy_node_t *node)
{
    for (sy_node_t *item = node;item != NULL;item = item->next)
    {
        int32_t r1 = sy_semantic_statement(item);
        if (r1 == -1)
        {
            return -1;
        }
    }
    return 0;
}

static int32_t
sy_semantic_note(sy_node_t *node)
{
    sy_node_carrier_t *carrier1 = (sy_node_carrier_t *)node->value;

    assert (carrier1->base->kind == NODE_KIND_ID);

    for (sy_node_t *node1 = node->previous; node1 != NULL;node1 = node1->previous)
    {
        sy_node_carrier_t *carrier2 = (sy_node_carrier_t *)node1->value;

        assert (carrier2->base->kind == NODE_KIND_ID);

        if (sy_semantic_idcmp(carrier1->base, carrier2->base) == 1)
        {
            sy_node_basic_t *basic1 = (sy_node_basic_t *)carrier1->base->value;

            sy_error_semantic_by_node(carrier1->base, "'%s' already defined, previous in (%lld:%lld)", 
                basic1->value, carrier2->base->position.line, carrier2->base->position.column);
            return -1;
        }
    }

    if (carrier1->base != NULL)
    {
        int32_t r1 = sy_semantic_expression(carrier1->base);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (carrier1->data != NULL)
    {
        int32_t r1 = sy_semantic_arguments(carrier1->data);
        if (r1 == -1)
        {
            return -1;
        }
    }

    return 0;
}

static int32_t
sy_semantic_notes(sy_node_t *node)
{
    sy_node_block_t *block = (sy_node_block_t *)node->value;

	for (sy_node_t *item = block->items;item != NULL;item = item->next)
    { 
        int32_t r1 = sy_semantic_note(item);
        if (r1 == -1)
        {
            return -1;
        }
    }
	return 0;
}

static int32_t
sy_semantic_fun(sy_node_t *node)
{
	sy_node_fun_t *fun1 = (sy_node_fun_t *)node->value;

    for (sy_node_t *node1 = node->parent; node1 != NULL;node1 = node1->parent)
    {
        if (node1->kind == NODE_KIND_CLASS)
        {
            sy_node_class_t *class1 = (sy_node_class_t *)node1->value;

            if (class1->generics != NULL)
            {
                sy_node_block_t *block1 = (sy_node_block_t *)class1->generics->value;

                for (sy_node_t *item1 = block1->items;item1 != NULL;item1 = item1->next)
                {
                    assert (item1->kind == NODE_KIND_GENERIC);

                    sy_node_generic_t *generic1 = (sy_node_generic_t *)item1->value;
                    if (sy_semantic_idcmp(fun1->key, generic1->key) == 1)
                    {
                        sy_node_basic_t *basic1 = (sy_node_basic_t *)fun1->key->value;

                        sy_error_semantic_by_node(fun1->key, "'%s' already defined, previous in (%lld:%lld)",
                            basic1->value, generic1->key->position.line, generic1->key->position.column);
                        return -1;
                    }
                }
            }
            
            if (class1->heritages != NULL)
            {
                sy_node_block_t *block1 = (sy_node_block_t *)class1->heritages->value;

                for (sy_node_t *item1 = block1->items;item1 != NULL;item1 = item1->next)
                {
                    assert (item1->kind == NODE_KIND_HERITAGE);

                    sy_node_heritage_t *heritage1 = (sy_node_heritage_t *)item1->value;

                    if (sy_semantic_idcmp(fun1->key, heritage1->key) == 1)
                    {
                        sy_node_basic_t *basic1 = (sy_node_basic_t *)fun1->key->value;

                        sy_error_semantic_by_node(fun1->key, "'%s' already defined, previous in (%lld:%lld)",
                            basic1->value, heritage1->key->position.line, heritage1->key->position.column);
                        return -1;
                    }
                }
            }
            

            for (sy_node_t *item1 = class1->block;item1 != NULL;item1 = item1->next)
            {
                if (item1->id == node->id)
                {
                    break;
                }

                if (item1->kind == NODE_KIND_CLASS)
                {
                    sy_node_class_t *class2 = (sy_node_class_t *)item1->value;
                    if (sy_semantic_idcmp(fun1->key, class2->key) == 1)
                    {
                        sy_node_basic_t *basic1 = (sy_node_basic_t *)fun1->key->value;

                        sy_error_semantic_by_node(fun1->key, "'%s' already defined, previous in (%lld:%lld)",
                            basic1->value, class2->key->position.line, class2->key->position.column);
                        return -1;
                    }
                }
                else
                if (item1->kind == NODE_KIND_FUN)
                {
                    sy_node_fun_t *fun2 = (sy_node_fun_t *)item1->value;

                    if (sy_semantic_idcmp(fun1->key, fun2->key) == 1)
                    {
                        sy_node_basic_t *basic1 = (sy_node_basic_t *)fun1->key->value;
                        sy_error_semantic_by_node(fun1->key, "'%s' already defined, previous in (%lld:%lld)",
                            basic1->value, fun2->key->position.line, fun2->key->position.column);
                        return -1;
                    }
                }
                else
                if (item1->kind == NODE_KIND_PROPERTY)
                {
                    sy_node_property_t *property1 = (sy_node_property_t *)item1->value;
                    if (sy_semantic_idcmp(fun1->key, property1->key) == 1)
                    {
                        sy_node_basic_t *basic1 = (sy_node_basic_t *)fun1->key->value;
                        sy_error_semantic_by_node(fun1->key, "'%s' already defined, previous in (%lld:%lld)",
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
        int32_t r1 = sy_semantic_notes(fun1->notes);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (fun1->generics != NULL)
    {
        int32_t r1 = sy_semantic_generics(fun1->generics);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (fun1->parameters != NULL)
    {
        int32_t r1 = sy_semantic_parameters(fun1->parameters);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (fun1->body != NULL)
    {
        int32_t r1 = sy_semantic_body(fun1->body);
        if (r1 == -1)
        {
            return -1;
        }
    }

	return 0;
}

static int32_t
sy_semantic_property(sy_node_t *node)
{
    sy_node_property_t *property1 = (sy_node_property_t *)node->value;
    
    for (sy_node_t *node1 = node->parent; node1 != NULL;node1 = node1->parent)
    {
        if (node1->kind == NODE_KIND_CLASS)
        {
            sy_node_class_t *class1 = (sy_node_class_t *)node1->value;

            if (class1->generics != NULL)
            {
                sy_node_block_t *block1 = (sy_node_block_t *)class1->generics->value;

                for (sy_node_t *item1 = block1->items;item1 != NULL;item1 = item1->next)
                {
                    assert (item1->kind == NODE_KIND_GENERIC);

                    sy_node_generic_t *generic1 = (sy_node_generic_t *)item1->value;
                    if (sy_semantic_idcmp(property1->key, generic1->key) == 1)
                    {
                        sy_node_basic_t *basic1 = (sy_node_basic_t *)property1->key->value;

                        sy_error_semantic_by_node(property1->key, "'%s' already defined, previous in (%lld:%lld)",
                            basic1->value, generic1->key->position.line, generic1->key->position.column);
                        return -1;
                    }
                }
            }
            
            if (class1->heritages != NULL)
            {
                sy_node_block_t *block1 = (sy_node_block_t *)class1->heritages->value;

                for (sy_node_t *item1 = block1->items;item1 != NULL;item1 = item1->next)
                {
                    assert (item1->kind == NODE_KIND_HERITAGE);

                    sy_node_heritage_t *heritage1 = (sy_node_heritage_t *)item1->value;

                    if (sy_semantic_idcmp(property1->key, heritage1->key) == 1)
                    {
                        sy_node_basic_t *basic1 = (sy_node_basic_t *)property1->key->value;

                        sy_error_semantic_by_node(property1->key, "'%s' already defined, previous in (%lld:%lld)",
                            basic1->value, heritage1->key->position.line, heritage1->key->position.column);
                        return -1;
                    }
                }
            }
            

            for (sy_node_t *item1 = class1->block;item1 != NULL;item1 = item1->next)
            {
                if (item1->id == node->id)
                {
                    break;
                }

                if (item1->kind == NODE_KIND_CLASS)
                {
                    sy_node_class_t *class2 = (sy_node_class_t *)item1->value;
                    if (sy_semantic_idcmp(property1->key, class2->key) == 1)
                    {
                        sy_node_basic_t *basic1 = (sy_node_basic_t *)property1->key->value;

                        sy_error_semantic_by_node(property1->key, "'%s' already defined, previous in (%lld:%lld)",
                            basic1->value, class2->key->position.line, class2->key->position.column);
                        return -1;
                    }
                }
                else
                if (item1->kind == NODE_KIND_FUN)
                {
                    sy_node_fun_t *fun1 = (sy_node_fun_t *)item1->value;

                    if (sy_semantic_idcmp(property1->key, fun1->key) == 1)
                    {
                        sy_node_basic_t *basic1 = (sy_node_basic_t *)property1->key->value;
                        sy_error_semantic_by_node(property1->key, "'%s' already defined, previous in (%lld:%lld)",
                            basic1->value, fun1->key->position.line, fun1->key->position.column);
                        return -1;
                    }
                }
                else
                if (item1->kind == NODE_KIND_PROPERTY)
                {
                    sy_node_property_t *property2 = (sy_node_property_t *)item1->value;
                    if (sy_semantic_idcmp(property1->key, property2->key) == 1)
                    {
                        sy_node_basic_t *basic1 = (sy_node_basic_t *)property1->key->value;
                        sy_error_semantic_by_node(property1->key, "'%s' already defined, previous in (%lld:%lld)",
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
        int32_t r1 = sy_semantic_notes(property1->notes);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (property1->type != NULL)
    {
        int32_t r1 = sy_semantic_expression(property1->type);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (property1->value != NULL)
    {
        int32_t r1 = sy_semantic_expression(property1->value);
        if (r1 == -1)
        {
            return -1;
        }
    }

    return 0;
}

static int32_t
sy_semantic_heritage(sy_node_t *node)
{
    sy_node_heritage_t *heritage1 = (sy_node_heritage_t *)node->value;

    for (sy_node_t *item1 = node->previous; item1 != NULL; item1 = item1->previous)
    {
        assert(item1->kind == NODE_KIND_HERITAGE);

        sy_node_heritage_t *heritage2 = (sy_node_heritage_t *)item1->value;
        if (sy_semantic_idcmp(heritage1->key, heritage2->key) == 1)
        {
            sy_node_basic_t *basic1 = (sy_node_basic_t *)heritage1->key->value;

            sy_error_semantic_by_node(heritage1->key, "'%s' already defined, previous in (%lld:%lld)",
                basic1->value, heritage2->key->position.line, heritage2->key->position.column);
            return -1;
        }
    }

    for (sy_node_t *node1 = node->parent; node1 != NULL;node1 = node1->parent)
    {
        if (node1->kind == NODE_KIND_CLASS)
        {
            sy_node_class_t *class1 = (sy_node_class_t *)node1->value;

            if (class1->generics != NULL)
            {
                for (sy_node_t *item1 = class1->generics;item1 != NULL;item1 = item1->next)
                {
                    assert (item1->kind == NODE_KIND_GENERIC);

                    sy_node_generic_t *generic1 = (sy_node_generic_t *)item1->value;
                    if (sy_semantic_idcmp(heritage1->key, generic1->key) == 1)
                    {
                        sy_node_basic_t *basic1 = (sy_node_basic_t *)heritage1->key->value;

                        sy_error_semantic_by_node(heritage1->key, "'%s' already defined, previous in (%lld:%lld)",
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
        int32_t r1 = sy_semantic_expression(heritage1->key);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (heritage1->type != NULL)
    {
        int32_t r1 = sy_semantic_expression(heritage1->type);
        if (r1 == -1)
        {
            return -1;
        }
    }

    return 0;
}

static int32_t
sy_semantic_heritages(sy_node_t *node)
{
    sy_node_block_t *block = (sy_node_block_t *)node->value;

	for (sy_node_t *item = block->items;item != NULL;item = item->next)
    {
        int32_t r1 = sy_semantic_heritage(item);
        if (r1 == -1)
        {
            return -1;
        }
    }
	return 0;
}

static int32_t
sy_semantic_class(sy_node_t *node)
{
	sy_node_class_t *class1 = (sy_node_class_t *)node->value;

    for (sy_node_t *node1 = node->parent, *subnode = node; node1 != NULL;subnode = node1, node1 = node1->parent)
    {
        if (node1->kind == NODE_KIND_MODULE)
        {
            sy_node_block_t *module1 = (sy_node_block_t *)node1->value;

            for (sy_node_t *item1 = module1->items;item1 != NULL;item1 = item1->next)
            { 
                if (item1->id == subnode->id)
                {
                    break;
                }
                
                if (item1->kind == NODE_KIND_USING)
                {
                    sy_node_using_t *using1 = (sy_node_using_t *)item1->value;
                    sy_node_block_t *packages1 = (sy_node_block_t *)using1->packages->value;
                    for (sy_node_t *item2 = packages1->items;item2 != NULL;item2 = item2->next)
                    {
                        sy_node_package_t *package1 = (sy_node_package_t *)item2->value;
                        if (sy_semantic_idcmp(package1->key, class1->key) == 1)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)class1->key->value;

                            sy_error_semantic_by_node(class1->key, "'%s' already defined, previous in (%lld:%lld)",
                                basic1->value, package1->key->position.line, package1->key->position.column);
                            return -1;
                        }
                    }
                }
                else
                if (item1->kind == NODE_KIND_CLASS)
                {
                    sy_node_class_t *class2 = (sy_node_class_t *)item1->value;
                    if (sy_semantic_idcmp(class1->key, class2->key) == 1)
                    {
                        sy_node_basic_t *basic1 = (sy_node_basic_t *)class1->key->value;

                        sy_error_semantic_by_node(class1->key, "'%s' already defined, previous in (%lld:%lld)",
                            basic1->value, class2->key->position.line, class2->key->position.column);
                        return -1;
                    }
                }
                else
                if (item1->kind == NODE_KIND_FOR)
                {
                    sy_node_for_t *for1 = (sy_node_for_t *)item1->value;
                    if (for1->key != NULL)
                    {
                        if (sy_semantic_idcmp(class1->key, for1->key) == 1)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)class1->key->value;

                            sy_error_semantic_by_node(class1->key, "'%s' already defined, previous in (%lld:%lld)",
                                basic1->value, for1->key->position.line, for1->key->position.column);
                            return -1;
                        }
                    }
                }
                else
                if (item1->kind == NODE_KIND_VAR)
                {
                    sy_node_var_t *var1 = (sy_node_var_t *)item1->value;
                    if (var1->key->kind == NODE_KIND_ID)
                    {
                        if (sy_semantic_idcmp(class1->key, var1->key) == 1)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)class1->key->value;

                            sy_error_semantic_by_node(class1->key, "'%s' already defined, previous in (%lld:%lld)",
                                basic1->value, var1->key->position.line, var1->key->position.column);
                            return -1;
                        }
                    }
                    else
                    {
                        for (sy_node_t *item1 = var1->key;item1 != NULL;item1 = item1->next)
                        {
                            assert(item1->kind == NODE_KIND_ENTITY);

                            sy_node_entity_t *entity1 = (sy_node_entity_t *)item1->value;
                            if (sy_semantic_idcmp(class1->key, entity1->key) == 1)
                            {
                                sy_node_basic_t *basic1 = (sy_node_basic_t *)class1->key->value;

                                sy_error_semantic_by_node(class1->key, "'%s' already defined, previous in (%lld:%lld)",
                                    basic1->value, entity1->key->position.line, entity1->key->position.column);
                                return -1;
                            }
                        }
                    }
                }
            }
            break;
        }
        else
        if (node1->kind == NODE_KIND_CLASS)
        {
            sy_node_class_t *class2 = (sy_node_class_t *)node1->value;

            if (class2->generics != NULL)
            {
                sy_node_block_t *block1 = (sy_node_block_t *)class1->generics->value;

                for (sy_node_t *item1 = block1->items;item1 != NULL;item1 = item1->next)
                {
                    assert (item1->kind == NODE_KIND_GENERIC);

                    sy_node_generic_t *generic1 = (sy_node_generic_t *)item1->value;

                    if (sy_semantic_idcmp(class1->key, generic1->key) == 1)
                    {
                        sy_node_basic_t *basic1 = (sy_node_basic_t *)class1->key->value;

                        sy_error_semantic_by_node(class1->key, "'%s' already defined, previous in (%lld:%lld)",
                            basic1->value, generic1->key->position.line, generic1->key->position.column);
                        return -1;
                    }
                }
            }
            
            if (class2->heritages != NULL)
            {
                sy_node_block_t *block1 = (sy_node_block_t *)class1->generics->value;

                for (sy_node_t *item1 = block1->items;item1 != NULL;item1 = item1->next)
                {
                    assert (item1->kind == NODE_KIND_HERITAGE);

                    sy_node_heritage_t *heritage1 = (sy_node_heritage_t *)item1->value;

                    if (sy_semantic_idcmp(class1->key, heritage1->key) == 1)
                    {
                        sy_node_basic_t *basic1 = (sy_node_basic_t *)class1->key->value;

                        sy_error_semantic_by_node(class1->key, "'%s' already defined, previous in (%lld:%lld)",
                            basic1->value, heritage1->key->position.line, heritage1->key->position.column);
                        return -1;
                    }
                }
            }
            
            for (sy_node_t *item1 = class2->block;item1 != NULL;item1 = item1->next)
            {
                if (item1->id == node->id)
                {
                    break;
                }

                if (item1->kind == NODE_KIND_CLASS)
                {
                    sy_node_class_t *class3 = (sy_node_class_t *)item1->value;
                    if (sy_semantic_idcmp(class1->key, class3->key) == 1)
                    {
                        sy_node_basic_t *basic1 = (sy_node_basic_t *)class1->key->value;

                        sy_error_semantic_by_node(class1->key, "'%s' already defined, previous in (%lld:%lld)",
                            basic1->value, class3->key->position.line, class3->key->position.column);
                        return -1;
                    }
                }
                else
                if (item1->kind == NODE_KIND_FUN)
                {
                    sy_node_fun_t *fun1 = (sy_node_fun_t *)item1->value;

                    if (sy_semantic_idcmp(class1->key, fun1->key) == 1)
                    {
                        sy_node_basic_t *basic1 = (sy_node_basic_t *)class1->key->value;
                        sy_error_semantic_by_node(class1->key, "'%s' already defined, previous in (%lld:%lld)",
                            basic1->value, fun1->key->position.line, fun1->key->position.column);
                        return -1;
                    }
                }
                else
                if (item1->kind == NODE_KIND_PROPERTY)
                {
                    sy_node_property_t *property1 = (sy_node_property_t *)item1->value;
                    if (sy_semantic_idcmp(class1->key, property1->key) == 1)
                    {
                        sy_node_basic_t *basic1 = (sy_node_basic_t *)class1->key->value;
                        sy_error_semantic_by_node(class1->key, "'%s' already defined, previous in (%lld:%lld)",
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
        int32_t r1 = sy_semantic_notes(class1->notes);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (class1->generics != NULL)
    {
        if ((class1->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
        {
            sy_error_semantic_by_node(class1->key, "Generic type in static class", 
                __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = sy_semantic_generics(class1->generics);
        if (r1 == -1)
        {
            return -1;
        }
    }

    if (class1->heritages != NULL)
    {
        if ((class1->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
        {
            sy_error_semantic_by_node(class1->key, "Generic type in inheritance", 
                __FILE__, __LINE__);
            return -1;
        }

        int32_t r1 = sy_semantic_heritages(class1->heritages);
        if (r1 == -1)
        {
            return -1;
        }
    }
    
    for (sy_node_t *item = class1->block;item != NULL;item = item->next)
    {
        if (item->kind == NODE_KIND_CLASS)
        {
            if ((class1->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
            {
                sy_node_class_t *class2 = (sy_node_class_t *)item->value;
                if ((class2->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                {
                    sy_error_semantic_by_node(class2->key, "Static class(%lld:%lld), members must be static", 
                        class1->key->position.line, class1->key->position.column);
                    return -1;
                }
            }

            int32_t r1 = sy_semantic_class(item);
            if (r1 == -1)
            {
                return -1;
            }
        }
        else
        if (item->kind == NODE_KIND_FUN)
        {
            if ((class1->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
            {
                sy_node_fun_t *fun1 = (sy_node_fun_t *)item->value;
                if ((fun1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                {
                    sy_error_semantic_by_node(fun1->key, "Static class(%lld:%lld), members must be static", 
                        class1->key->position.line, class1->key->position.column);
                    return -1;
                }
            }

            int32_t r1 = sy_semantic_fun(item);
            if (r1 == -1)
            {
                return -1;
            }
        }
        else
        if (item->kind == NODE_KIND_PROPERTY)
        {
            if ((class1->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
            {
                sy_node_property_t *property1 = (sy_node_property_t *)item->value;
                if ((property1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
                {
                    sy_error_semantic_by_node(property1->key, "Static class(%lld:%lld), members must be static", 
                        class1->key->position.line, class1->key->position.column);
                    return -1;
                }
            }

            sy_node_property_t *property1 = (sy_node_property_t *)item->value;
            if ((property1->flag & SYNTAX_MODIFIER_STATIC) != SYNTAX_MODIFIER_STATIC)
            {
                int32_t r1 = sy_semantic_property(item);
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
sy_semantic_package(sy_node_t *node)
{
    sy_node_package_t *package1 = (sy_node_package_t *)node->value;

    for (sy_node_t *item1 = node->previous; item1 != NULL; item1 = item1->previous)
    {
        assert(item1->kind == NODE_KIND_PACKAGE);

        sy_node_package_t *package2 = (sy_node_package_t *)item1->value;
        if (sy_semantic_idcmp(package1->key, package2->key) == 1)
        {
            sy_node_basic_t *basic1 = (sy_node_basic_t *)package1->key->value;

            sy_error_semantic_by_node(package1->key, "'%s' already defined, previous in (%lld:%lld)",
                basic1->value, package2->key->position.line, package2->key->position.column);
            return -1;
        }
    }

    for (sy_node_t *node1 = node->parent, *subnode = node; node1 != NULL;subnode = node1, node1 = node1->parent)
    {
        if (node1->kind == NODE_KIND_MODULE)
        {
            sy_node_block_t *module1 = (sy_node_block_t *)node1->value;
            
            for (sy_node_t *item1 = module1->items;item1 != NULL;item1 = item1->next)
            {   
                if (item1->id == subnode->id)
                {
                    break;
                }

                if (item1->kind == NODE_KIND_USING)
                {
                    sy_node_using_t *using1 = (sy_node_using_t *)item1->value;
                    sy_node_block_t *packages1 = (sy_node_block_t *)using1->packages->value;
                    for (sy_node_t *item2 = packages1->items;item2 != NULL;item2 = item2->next)
                    {
                        assert(item2->kind == NODE_KIND_PACKAGE);

                        sy_node_package_t *package2 = (sy_node_package_t *)item1->value;
                        if (sy_semantic_idcmp(package1->key, package2->key) == 1)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)package1->key->value;

                            sy_error_semantic_by_node(package1->key, "'%s' already defined, previous in (%lld:%lld)",
                                basic1->value, package2->key->position.line, package2->key->position.column);
                            return -1;
                        }
                    }
                }
                else
                if (item1->kind == NODE_KIND_CLASS)
                {
                    sy_node_class_t *class1 = (sy_node_class_t *)item1->value;

                    if (sy_semantic_idcmp(package1->key, class1->key) == 1)
                    {
                        sy_node_basic_t *basic1 = (sy_node_basic_t *)package1->key->value;

                        sy_error_semantic_by_node(package1->key, "'%s' already defined, previous in (%lld:%lld)",
                            basic1->value, class1->key->position.line, class1->key->position.column);
                        return -1;
                    }
                }
                else
                if (item1->kind == NODE_KIND_FOR)
                {
                    sy_node_for_t *for1 = (sy_node_for_t *)item1->value;
                    if (for1->key != NULL)
                    {
                        if (sy_semantic_idcmp(package1->key, for1->key) == 1)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)package1->key->value;

                            sy_error_semantic_by_node(package1->key, "'%s' already defined, previous in (%lld:%lld)",
                                basic1->value, for1->key->position.line, for1->key->position.column);
                            return -1;
                        }
                    }
                }
                else
                if (item1->kind == NODE_KIND_VAR)
                {
                    sy_node_var_t *var1 = (sy_node_var_t *)item1->value;
                    if (var1->key->kind == NODE_KIND_ID)
                    {
                        if (sy_semantic_idcmp(package1->key, var1->key) == 1)
                        {
                            sy_node_basic_t *basic1 = (sy_node_basic_t *)package1->key->value;

                            sy_error_semantic_by_node(package1->key, "'%s' already defined, previous in (%lld:%lld)",
                                basic1->value, var1->key->position.line, var1->key->position.column);
                            return -1;
                        }
                    }
                    else
                    {
                        for (sy_node_t *item1 = var1->key;item1 != NULL;item1 = item1->next)
                        {
                            assert(item1->kind == NODE_KIND_ENTITY);

                            sy_node_entity_t *entity1 = (sy_node_entity_t *)item1->value;

                            if (sy_semantic_idcmp(package1->key, entity1->key) == 1)
                            {
                                sy_node_basic_t *basic1 = (sy_node_basic_t *)package1->key->value;

                                sy_error_semantic_by_node(package1->key, "'%s' already defined, previous in (%lld:%lld)",
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
        int32_t r1 = sy_semantic_expression(package1->value);
        if (r1 == -1)
        {
            return -1;
        }
    }

    return 0;
}

static int32_t
sy_semantic_packages(sy_node_t *node)
{
    sy_node_block_t *block = (sy_node_block_t *)node->value;

    for (sy_node_t *item = block->items;item != NULL;item = item->next)
    {
        int32_t r1 = sy_semantic_package(item);
        if (r1 == -1)
        {
            return -1;
        }
    }

	return 0;
}

static int32_t
sy_semantic_using(sy_node_t *node)
{
	sy_node_using_t *using1 = (sy_node_using_t *)node->value;

	if (using1->packages != NULL)
    {
        int32_t r1 = sy_semantic_packages(using1->packages);
        if (r1 == -1)
        {
            return -1;
        }
    }
    
    sy_node_t *node1 = using1->path;
    sy_node_basic_t *basic1 = (sy_node_basic_t *)node1->value;

    sy_module_entry_t *module_entry = sy_module_load(basic1->value);
    if (module_entry == ERROR)
    {
        sy_error_semantic_by_node(node, "Error:incapable of loading the modulus");
        return -1;
    }

	return 0;
}

int32_t
sy_semantic_module(sy_node_t *node)
{
	sy_node_block_t *module = (sy_node_block_t *)node->value;

    for (sy_node_t *item = module->items;item != NULL;item = item->next)
    {
        if (item->kind == NODE_KIND_USING)
        {
            int32_t r1 = sy_semantic_using(item);
            if (r1 == -1)
            {
                return -1;
            }
        }
        else
        if (item->kind == NODE_KIND_CLASS)
        {
            int32_t r1 = sy_semantic_class(item);
            if (r1 == -1)
            {
                return -1;
            }
        }
        else
        if (item->kind == NODE_KIND_IF)
        {
            int32_t r1 = sy_semantic_if(item);
            if (r1 == -1)
            {
                return -1;
            }
        }
        else 
        if (item->kind == NODE_KIND_FOR)
        {
            int32_t r1 = sy_semantic_for(item);
            if (r1 == -1)
            {
                return -1;
            }
        }
        else
        if (item->kind == NODE_KIND_TRY)
        {
            int32_t r1 = sy_semantic_try(item);
            if (r1 == -1)
            {
                return -1;
            }
        }
        else
        if (item->kind == NODE_KIND_VAR)
        {
            int32_t r1 = sy_semantic_var(item);
            if (r1 == -1)
            {
                return -1;
            }
        }

    }

	return 0;
}
