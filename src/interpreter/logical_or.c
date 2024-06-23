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
not_execute_lor(not_node_t *node, not_record_t *left, not_record_t *right, not_node_t *applicant)
{
    if (left->kind == RECORD_KIND_UNDEFINED)
    {
        return right;
    }
    else if (left->kind == RECORD_KIND_NAN)
    {
        return right;
    }
    else if (left->kind == RECORD_KIND_INT)
    {
        int32_t a_is_nonzero = mpz_cmp_ui(*(mpz_t *)(left->value), 0) != 0;
        if (a_is_nonzero)
        {
            if (right)
            {
                if (not_record_link_decrease(right) < 0)
                {
                    return NOT_PTR_ERROR;
                }
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
        int a_is_nonzero = mpf_cmp_ui(*(mpf_t *)(left->value), 0) != 0;
        if (a_is_nonzero)
        {
            if (right)
            {
                if (not_record_link_decrease(right) < 0)
                {
                    return NOT_PTR_ERROR;
                }
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
        int a_is_nonzero = (*(char *)(left->value)) != 0;
        if (a_is_nonzero)
        {
            if (right)
            {
                if (not_record_link_decrease(right) < 0)
                {
                    return NOT_PTR_ERROR;
                }
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
        int a_is_nonzero = strcmp((char *)(left->value), "") != 0;
        if (a_is_nonzero)
        {
            if (right)
            {
                if (not_record_link_decrease(right) < 0)
                {
                    return NOT_PTR_ERROR;
                }
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
    else if (left->kind == RECORD_KIND_OBJECT)
    {
        if (right)
        {
            if (not_record_link_decrease(right) < 0)
            {
                return NOT_PTR_ERROR;
            }
        }
        return left;
    }
    else if (left->kind == RECORD_KIND_TUPLE)
    {
        if (right)
        {
            if (not_record_link_decrease(right) < 0)
            {
                return NOT_PTR_ERROR;
            }
        }
        return left;
    }
    else if (left->kind == RECORD_KIND_TYPE)
    {
        if (right)
        {
            if (not_record_link_decrease(right) < 0)
            {
                return NOT_PTR_ERROR;
            }
        }
        return left;
    }
    else if (left->kind == RECORD_KIND_STRUCT)
    {
        if (right)
        {
            if (not_record_link_decrease(right) < 0)
            {
                return NOT_PTR_ERROR;
            }
        }
        return left;
    }
    else if (left->kind == RECORD_KIND_NULL)
    {
        int a_is_nonzero = (*(int64_t *)(left->value)) != 0;
        if (a_is_nonzero)
        {
            if (right)
            {
                if (not_record_link_decrease(right) < 0)
                {
                    return NOT_PTR_ERROR;
                }
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

    return not_record_make_undefined();
}

not_record_t *
not_logical_or(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin)
{
    if (node->kind == NODE_KIND_LOR)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;
        not_record_t *left = not_logical_or(binary->left, strip, applicant, origin);
        if (left == NOT_PTR_ERROR)
        {
            return NOT_PTR_ERROR;
        }

        not_record_t *right = not_logical_and(binary->right, strip, applicant, origin);
        if (right == NOT_PTR_ERROR)
        {
            not_record_link_decrease(left);
            return NOT_PTR_ERROR;
        }

        return not_execute_lor(node, left, right, applicant);
    }
    else
    {
        return not_logical_and(node, strip, applicant, origin);
    }
}