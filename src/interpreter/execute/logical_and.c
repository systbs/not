
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#include <gmp.h>
#include <jansson.h>

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
sy_execute_land(sy_node_t *node, sy_record_t *left, sy_record_t *right, sy_node_t *applicant)
{
    if (left->kind == RECORD_KIND_UNDEFINED)
    {
        if (right)
        {
            if (sy_record_link_decrease(right) < 0)
            {
                return ERROR;
            }
        }
        return left;
    }
    else if (left->kind == RECORD_KIND_NAN)
    {
        if (right)
        {
            if (sy_record_link_decrease(right) < 0)
            {
                return ERROR;
            }
        }
        return left;
    }
    else if (left->kind == RECORD_KIND_INT)
    {
        int32_t a_is_zero = mpz_cmp_ui(*(mpz_t *)(left->value), 0) == 0;
        if (a_is_zero)
        {
            if (right)
            {
                if (sy_record_link_decrease(right) < 0)
                {
                    return ERROR;
                }
            }
            return left;
        }
        else
        {
            if (sy_record_link_decrease(left) < 0)
            {
                return ERROR;
            }
            return right;
        }
    }
    else if (left->kind == RECORD_KIND_FLOAT)
    {
        int a_is_zero = mpf_cmp_ui(*(mpf_t *)(left->value), 0) == 0;
        if (a_is_zero)
        {
            if (right)
            {
                if (sy_record_link_decrease(right) < 0)
                {
                    return ERROR;
                }
            }
            return left;
        }
        else
        {
            if (sy_record_link_decrease(left) < 0)
            {
                return ERROR;
            }
            return right;
        }
    }
    else if (left->kind == RECORD_KIND_CHAR)
    {
        int a_is_zero = (*(char *)(left->value)) == 0;
        if (a_is_zero)
        {
            if (right)
            {
                if (sy_record_link_decrease(right) < 0)
                {
                    return ERROR;
                }
            }
            return left;
        }
        else
        {
            if (sy_record_link_decrease(left) < 0)
            {
                return ERROR;
            }
            return right;
        }
    }
    else if (left->kind == RECORD_KIND_STRING)
    {
        int a_is_zero = strcmp((char *)(left->value), "") == 0;
        if (a_is_zero)
        {
            if (right)
            {
                if (sy_record_link_decrease(right) < 0)
                {
                    return ERROR;
                }
            }
            return left;
        }
        else
        {
            if (sy_record_link_decrease(left) < 0)
            {
                return ERROR;
            }
            return right;
        }
    }
    else if (left->kind == RECORD_KIND_OBJECT)
    {
        if (sy_record_link_decrease(left) < 0)
        {
            return ERROR;
        }
        return right;
    }
    else if (left->kind == RECORD_KIND_TUPLE)
    {
        if (sy_record_link_decrease(left) < 0)
        {
            return ERROR;
        }
        return right;
    }
    else if (left->kind == RECORD_KIND_TYPE)
    {
        if (sy_record_link_decrease(left) < 0)
        {
            return ERROR;
        }
        return right;
    }
    else if (left->kind == RECORD_KIND_STRUCT)
    {
        if (sy_record_link_decrease(left) < 0)
        {
            return ERROR;
        }
        return right;
    }
    else if (left->kind == RECORD_KIND_NULL)
    {
        int a_is_zero = (*(int64_t *)(left->value)) == 0;
        if (a_is_zero)
        {
            if (right)
            {
                if (sy_record_link_decrease(right) < 0)
                {
                    return ERROR;
                }
            }
            return left;
        }
        else
        {
            if (sy_record_link_decrease(left) < 0)
            {
                return ERROR;
            }
            return right;
        }
    }

    return sy_record_make_undefined();
}

sy_record_t *
sy_execute_logical_and(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    if (node->kind == NODE_KIND_LAND)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        sy_record_t *left = sy_execute_logical_and(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return ERROR;
        }

        sy_record_t *right = sy_execute_bitwise_or(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            sy_record_link_decrease(left);
            return ERROR;
        }

        return sy_execute_land(node, left, right, applicant);
    }
    else
    {
        return sy_execute_bitwise_or(node, strip, applicant, origin);
    }
}