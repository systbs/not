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
#include <jansson.h>
#include <ffi.h>

#include "../types/types.h"
#include "../container/queue.h"
#include "../token/position.h"
#include "../token/token.h"
#include "../ast/node.h"
#include "../utils/utils.h"
#include "../utils/path.h"
#include "../error.h"
#include "../mutex.h"
#include "../memory.h"
#include "../config.h"
#include "../scanner/scanner.h"
#include "../parser/syntax/syntax.h"
#include "record.h"
#include "../repository.h"
#include "../interpreter.h"
#include "../thread.h"
#include "symbol_table.h"
#include "strip.h"
#include "entry.h"
#include "helper.h"
#include "execute.h"

not_record_t *
not_execute_land(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant)
{
    if (left->null || left->undefined || left->nan)
    {
        if (not_record_link_decrease(right) < 0)
        {
            return NOT_PTR_ERROR;
        }
        return left;
    }
    else if (left->kind == RECORD_KIND_INT)
    {
        int32_t a_is_zero = mpz_cmp_ui(*(mpz_t *)(left->value), 0) == 0;
        if (a_is_zero)
        {
            if (not_record_link_decrease(right) < 0)
            {
                return NOT_PTR_ERROR;
            }
            return left;
        }
        else
        {
            if (not_record_link_decrease(left) < 0)
            {
                return NOT_PTR_ERROR;
            }
            return right;
        }
    }
    else if (left->kind == RECORD_KIND_FLOAT)
    {
        int a_is_zero = mpf_cmp_ui(*(mpf_t *)(left->value), 0) == 0;
        if (a_is_zero)
        {
            if (not_record_link_decrease(right) < 0)
            {
                return NOT_PTR_ERROR;
            }
            return left;
        }
        else
        {
            if (not_record_link_decrease(left) < 0)
            {
                return NOT_PTR_ERROR;
            }
            return right;
        }
    }
    else if (left->kind == RECORD_KIND_CHAR)
    {
        int a_is_zero = (*(char *)(left->value)) == 0;
        if (a_is_zero)
        {
            if (not_record_link_decrease(right) < 0)
            {
                return NOT_PTR_ERROR;
            }
            return left;
        }
        else
        {
            if (not_record_link_decrease(left) < 0)
            {
                return NOT_PTR_ERROR;
            }
            return right;
        }
    }
    else if (left->kind == RECORD_KIND_STRING)
    {
        int a_is_zero = strcmp((char *)(left->value), "") == 0;
        if (a_is_zero)
        {
            if (not_record_link_decrease(right) < 0)
            {
                return NOT_PTR_ERROR;
            }
            return left;
        }
        else
        {
            if (not_record_link_decrease(left) < 0)
            {
                return NOT_PTR_ERROR;
            }
            return right;
        }
    }

    if (not_record_link_decrease(right) < 0)
    {
        return NOT_PTR_ERROR;
    }
    return left;
}

not_record_t *
not_logical_and(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin)
{
    if (node->kind == NODE_KIND_LAND)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;
        not_record_t *left = not_logical_and(binary->left, strip, applicant, origin);
        if (left == NOT_PTR_ERROR)
        {
            return NOT_PTR_ERROR;
        }

        not_record_t *right = not_bitwise_or(binary->right, strip, applicant, origin);
        if (right == NOT_PTR_ERROR)
        {
            not_record_link_decrease(left);
            return NOT_PTR_ERROR;
        }

        return not_execute_land(node, left, right, applicant);
    }
    else
    {
        return not_bitwise_or(node, strip, applicant, origin);
    }
}