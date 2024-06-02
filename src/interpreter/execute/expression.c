#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#include <gmp.h>
#include <mpfr.h>
#include <stdint.h>
#include <float.h>

#include "../../types/types.h"
#include "../../container/queue.h"
#include "../../token/position.h"
#include "../../token/token.h"
#include "../../scanner/scanner.h"
#include "../../ast/node.h"
#include "../../utils/utils.h"
#include "../../utils/path.h"
#include "../../parser/syntax/syntax.h"
#include "../../error.h"
#include "../../mutex.h"
#include "../record.h"
#include "../garbage.h"
#include "../symbol_table.h"
#include "../strip.h"
#include "execute.h"

sy_record_t *
sy_execute_expression(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    sy_node_binary_t *binary;
    sy_node_unary_t *unary;
    sy_record_t *left, *right, *record;

    switch (node->kind)
    {
    case NODE_KIND_TILDE:
        unary = (sy_node_unary_t *)node->value;
        right = sy_execute_expression(unary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            return ERROR;
        }

        left = sy_record_make_int64(-1);
        if (left == ERROR)
        {
            return ERROR;
        }

        record = sy_execute_xor(node, left, right);

        if (left->link == 0)
        {
            if (sy_record_destroy(left) < 0)
            {
                return ERROR;
            }
        }

        if (right->link == 0)
        {
            if (sy_record_destroy(right) < 0)
            {
                return ERROR;
            }
        }

        return record;

    case NODE_KIND_POS:
        unary = (sy_node_unary_t *)node->value;
        right = sy_execute_expression(unary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            return ERROR;
        }

        left = sy_record_make_int64(0);
        if (left == ERROR)
        {
            return ERROR;
        }

        record = sy_execute_plus(node, left, right);

        if (left->link == 0)
        {
            if (sy_record_destroy(left) < 0)
            {
                return ERROR;
            }
        }

        if (right->link == 0)
        {
            if (sy_record_destroy(right) < 0)
            {
                return ERROR;
            }
        }

        return record;

    case NODE_KIND_NEG:
        unary = (sy_node_unary_t *)node->value;
        right = sy_execute_expression(unary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            return ERROR;
        }

        left = sy_record_make_int64(0);
        if (left == ERROR)
        {
            return ERROR;
        }

        record = sy_execute_minus(node, left, right);

        if (left->link == 0)
        {
            if (sy_record_destroy(left) < 0)
            {
                return ERROR;
            }
        }

        if (right->link == 0)
        {
            if (sy_record_destroy(right) < 0)
            {
                return ERROR;
            }
        }

        return record;

    case NODE_KIND_NOT:
        unary = (sy_node_unary_t *)node->value;
        right = sy_execute_expression(unary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            return ERROR;
        }

        left = sy_record_make_int64(0);
        if (left == ERROR)
        {
            return ERROR;
        }

        record = sy_record_make_int32(sy_execute_truthy_eq(left, right));

        if (left->link == 0)
        {
            if (sy_record_destroy(left) < 0)
            {
                return ERROR;
            }
        }

        if (right->link == 0)
        {
            if (sy_record_destroy(right) < 0)
            {
                return ERROR;
            }
        }

        return record;

    case NODE_KIND_POW:
        binary = (sy_node_binary_t *)node->value;
        left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return ERROR;
        }

        right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            return ERROR;
        }

        record = sy_execute_pow(node, left, right);
        
        if (left->link == 0)
        {
            if (sy_record_destroy(left) < 0)
            {
                return ERROR;
            }
        }

        if (right->link == 0)
        {
            if (sy_record_destroy(right) < 0)
            {
                return ERROR;
            }
        }

        return record;

    case NODE_KIND_MUL:
        binary = (sy_node_binary_t *)node->value;
        left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return ERROR;
        }
       right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            return ERROR;
        }
       record = sy_execute_mul(node, left, right);
        if (left->link == 0)
        {
            if (sy_record_destroy(left) < 0)
            {
                return ERROR;
            }
        }
       if (right->link == 0)
        {
            if (sy_record_destroy(right) < 0)
            {
                return ERROR;
            }
        }
       return record;

    case NODE_KIND_DIV:
        binary = (sy_node_binary_t *)node->value;
        left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return ERROR;
        }
       right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            return ERROR;
        }
        
        record = sy_execute_div(node, left, right);
        if (left->link == 0)
        {
            if (sy_record_destroy(left) < 0)
            {
                return ERROR;
            }
        }
       if (right->link == 0)
        {
            if (sy_record_destroy(right) < 0)
            {
                return ERROR;
            }
        }
       return record;

    case NODE_KIND_MOD:
        binary = (sy_node_binary_t *)node->value;
        left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return ERROR;
        }
       right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            return ERROR;
        }
        
        record = sy_execute_mod(node, left, right);
        if (left->link == 0)
        {
            if (sy_record_destroy(left) < 0)
            {
                return ERROR;
            }
        }
       if (right->link == 0)
        {
            if (sy_record_destroy(right) < 0)
            {
                return ERROR;
            }
        }
       return record;

    case NODE_KIND_EPI:
        binary = (sy_node_binary_t *)node->value;
        left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return ERROR;
        }
       right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            return ERROR;
        }
       record = sy_execute_epi(node, left, right);
        if (left->link == 0)
        {
            if (sy_record_destroy(left) < 0)
            {
                return ERROR;
            }
        }
       if (right->link == 0)
        {
            if (sy_record_destroy(right) < 0)
            {
                return ERROR;
            }
        }
       return record;

    case NODE_KIND_PLUS:
        binary = (sy_node_binary_t *)node->value;
        left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return ERROR;
        }

        right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            return ERROR;
        }

        record = sy_execute_plus(node, left, right);

        if (left->link == 0)
        {
            if (sy_record_destroy(left) < 0)
            {
                return ERROR;
            }
        }

        if (right->link == 0)
        {
            if (sy_record_destroy(right) < 0)
            {
                return ERROR;
            }
        }

        return record;

    case NODE_KIND_MINUS:
        binary = (sy_node_binary_t *)node->value;
        left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return ERROR;
        }

        right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            return ERROR;
        }

        record = sy_execute_minus(node, left, right);
        if (left->link == 0)
        {
            if (sy_record_destroy(left) < 0)
            {
                return ERROR;
            }
        }

        if (right->link == 0)
        {
            if (sy_record_destroy(right) < 0)
            {
                return ERROR;
            }
        }

        return record;

    case NODE_KIND_SHR:
        binary = (sy_node_binary_t *)node->value;
        left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return ERROR;
        }

        right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            return ERROR;
        }

        record = sy_execute_shr(node, left, right);

        if (left->link == 0)
        {
            if (sy_record_destroy(left) < 0)
            {
                return ERROR;
            }
        }

        if (right->link == 0)
        {
            if (sy_record_destroy(right) < 0)
            {
                return ERROR;
            }
        }

        return record;

    case NODE_KIND_SHL:
        binary = (sy_node_binary_t *)node->value;
        left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return ERROR;
        }

        right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            return ERROR;
        }

        record = sy_execute_shl(node, left, right);

        if (left->link == 0)
        {
            if (sy_record_destroy(left) < 0)
            {
                return ERROR;
            }
        }

        if (right->link == 0)
        {
            if (sy_record_destroy(right) < 0)
            {
                return ERROR;
            }
        }

        return record;

    case NODE_KIND_LT:
        binary = (sy_node_binary_t *)node->value;
        left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return ERROR;
        }

        right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            return ERROR;
        }
        

        record = sy_record_make_int32(sy_execute_truthy_lt(left, right));
        
        if (left->link == 0)
        {
            if (sy_record_destroy(left) < 0)
            {
                return ERROR;
            }
        }

        if (right->link == 0)
        {
            if (sy_record_destroy(right) < 0)
            {
                return ERROR;
            }
        }

        return record;

    case NODE_KIND_LE:
        binary = (sy_node_binary_t *)node->value;
        left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return ERROR;
        }

        right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            return ERROR;
        }

        record = sy_record_make_int32(sy_execute_truthy_le(left, right));

        if (left->link == 0)
        {
            if (sy_record_destroy(left) < 0)
            {
                return ERROR;
            }
        }

        if (right->link == 0)
        {
            if (sy_record_destroy(right) < 0)
            {
                return ERROR;
            }
        }

        return record;

    case NODE_KIND_GT:
        binary = (sy_node_binary_t *)node->value;
        left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return ERROR;
        }

        right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            return ERROR;
        }

        record = sy_record_make_int32(!sy_execute_truthy_le(left, right));

        if (left->link == 0)
        {
            if (sy_record_destroy(left) < 0)
            {
                return ERROR;
            }
        }

        if (right->link == 0)
        {
            if (sy_record_destroy(right) < 0)
            {
                return ERROR;
            }
        }

        return record;

    case NODE_KIND_GE:
        binary = (sy_node_binary_t *)node->value;
        left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return ERROR;
        }

        right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            return ERROR;
        }

        record = sy_record_make_int32(!sy_execute_truthy_lt(left, right));

        if (left->link == 0)
        {
            if (sy_record_destroy(left) < 0)
            {
                return ERROR;
            }
        }

        if (right->link == 0)
        {
            if (sy_record_destroy(right) < 0)
            {
                return ERROR;
            }
        }

        return record;

    case NODE_KIND_EQ:
        binary = (sy_node_binary_t *)node->value;
        left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return ERROR;
        }

        right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            return ERROR;
        }

        record = sy_record_make_int32(sy_execute_truthy_eq(left, right));

        if (left->link == 0)
        {
            if (sy_record_destroy(left) < 0)
            {
                return ERROR;
            }
        }

        if (right->link == 0)
        {
            if (sy_record_destroy(right) < 0)
            {
                return ERROR;
            }
        }

        return record;

    case NODE_KIND_NEQ:
        binary = (sy_node_binary_t *)node->value;
        left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return ERROR;
        }

        right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            return ERROR;
        }

        record = sy_record_make_int32(!sy_execute_truthy_eq(left, right));

        if (left->link == 0)
        {
            if (sy_record_destroy(left) < 0)
            {
                return ERROR;
            }
        }

        if (right->link == 0)
        {
            if (sy_record_destroy(right) < 0)
            {
                return ERROR;
            }
        }

        return record;

    case NODE_KIND_AND:
        binary = (sy_node_binary_t *)node->value;
        left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {            
            return ERROR;
        }

        right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {            
            return ERROR;
        }

        record = sy_execute_and(node, left, right);
        
        if (left->link == 0)
        {
            if (sy_record_destroy(left) < 0)
            {
                return ERROR;
            }
        }

        if (right->link == 0)
        {
            if (sy_record_destroy(right) < 0)
            {
                return ERROR;
            }
        }

        return record;

    case NODE_KIND_XOR:
        binary = (sy_node_binary_t *)node->value;
        left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return ERROR;
        }

        right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            return ERROR;
        }

        record = sy_execute_xor(node, left, right);

        if (left->link == 0)
        {
            if (sy_record_destroy(left) < 0)
            {
                return ERROR;
            }
        }

        if (right->link == 0)
        {
            if (sy_record_destroy(right) < 0)
            {
                return ERROR;
            }
        }

        return record;

    case NODE_KIND_OR:
        binary = (sy_node_binary_t *)node->value;
        left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return ERROR;
        }

        right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            return ERROR;
        }

        record = sy_execute_or(node, left, right);

        if (left->link == 0)
        {
            if (sy_record_destroy(left) < 0)
            {
                return ERROR;
            }
        }

        if (right->link == 0)
        {
            if (sy_record_destroy(right) < 0)
            {
                return ERROR;
            }
        }

        return record;

    case NODE_KIND_LAND:
        binary = (sy_node_binary_t *)node->value;
        left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return ERROR;
        }

        right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            return ERROR;
        }

        return sy_execute_land(node, left, right);

    case NODE_KIND_LOR:
        binary = (sy_node_binary_t *)node->value;
        left = sy_execute_expression(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return ERROR;
        }

        right = sy_execute_expression(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            return ERROR;
        }

        return sy_execute_lor(node, left, right);
    default:
        return sy_execute_postfix(node, strip, applicant, origin);
    }
}