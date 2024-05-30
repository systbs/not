
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#include <gmp.h>

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
sy_execute_logical_or(sy_node_t *node, sy_strip_t *strip)
{
    if (node->kind == NODE_KIND_LOR)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        sy_record_t *left = sy_execute_logical_or(binary->left, strip);
        if (left == ERROR)
        {
            return ERROR;
        }

        sy_record_t *right = sy_execute_logical_and(binary->right, strip);
        if (right == ERROR)
        {
            return ERROR;
        }

        if (left == NULL)
        {
            return right;
        }
        else
        if (left == NAN)
        {
            return right;
        }
        else
        if (left->kind == RECORD_KIND_INT8)
        {
            int a_is_nonzero = (*(int8_t *)(left->value)) != 0;
            if (a_is_nonzero)
            {
                if (right)
                {
                    if (right->reference == 0)
                    {
                        if (sy_record_destroy(right) < 0)
                        {
                            return ERROR;
                        }
                    }
                }
                return left;
            }
            else
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }
                return right;
            }
        }
        else
        if (left->kind == RECORD_KIND_INT16)
        {
            int a_is_nonzero = (*(int16_t *)(left->value)) != 0;
            if (a_is_nonzero)
            {
                if (right)
                {
                    if (right->reference == 0)
                    {
                        if (sy_record_destroy(right) < 0)
                        {
                            return ERROR;
                        }
                    }
                }
                return left;
            }
            else
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }
                return right;
            }
        }
        else
        if (left->kind == RECORD_KIND_INT32)
        {
            int a_is_nonzero = (*(int32_t *)(left->value)) != 0;
            if (a_is_nonzero)
            {
                if (right)
                {
                    if (right->reference == 0)
                    {
                        if (sy_record_destroy(right) < 0)
                        {
                            return ERROR;
                        }
                    }
                }
                return left;
            }
            else
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }
                return right;
            }
        }
        else
        if (left->kind == RECORD_KIND_INT64)
        {
            int a_is_nonzero = (*(int64_t *)(left->value)) != 0;
            if (a_is_nonzero)
            {
                if (right)
                {
                    if (right->reference == 0)
                    {
                        if (sy_record_destroy(right) < 0)
                        {
                            return ERROR;
                        }
                    }
                }
                return left;
            }
            else
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }
                return right;
            }
        }
        else
        if (left->kind == RECORD_KIND_UINT8)
        {
            int a_is_nonzero = (*(uint8_t *)(left->value)) != 0;
            if (a_is_nonzero)
            {
                if (right)
                {
                    if (right->reference == 0)
                    {
                        if (sy_record_destroy(right) < 0)
                        {
                            return ERROR;
                        }
                    }
                }
                return left;
            }
            else
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }
                return right;
            }
        }
        else
        if (left->kind == RECORD_KIND_UINT16)
        {
            int a_is_nonzero = (*(uint16_t *)(left->value)) != 0;
            if (a_is_nonzero)
            {
                if (right)
                {
                    if (right->reference == 0)
                    {
                        if (sy_record_destroy(right) < 0)
                        {
                            return ERROR;
                        }
                    }
                }
                return left;
            }
            else
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }
                return right;
            }
        }
        else
        if (left->kind == RECORD_KIND_UINT32)
        {
            int a_is_nonzero = (*(uint32_t *)(left->value)) != 0;
            if (a_is_nonzero)
            {
                if (right)
                {
                    if (right->reference == 0)
                    {
                        if (sy_record_destroy(right) < 0)
                        {
                            return ERROR;
                        }
                    }
                }
                return left;
            }
            else
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }
                return right;
            }
        }
        else
        if (left->kind == RECORD_KIND_UINT64)
        {
            int a_is_nonzero = (*(uint64_t *)(left->value)) != 0;
            if (a_is_nonzero)
            {
                if (right)
                {
                    if (right->reference == 0)
                    {
                        if (sy_record_destroy(right) < 0)
                        {
                            return ERROR;
                        }
                    }
                }
                return left;
            }
            else
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }
                return right;
            }
        }
        else
        if (left->kind == RECORD_KIND_BIGINT)
        {
            int32_t a_is_nonzero = mpz_cmp_ui(*(mpz_t *)(left->value), 0) != 0;
            if (a_is_nonzero)
            {
                if (right)
                {
                    if (right->reference == 0)
                    {
                        if (sy_record_destroy(right) < 0)
                        {
                            return ERROR;
                        }
                    }
                }
                return left;
            }
            else
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }
                return right;
            }
        }
        else
        if (left->kind == RECORD_KIND_FLOAT32)
        {
            int a_is_nonzero = (*(float *)(left->value)) != 0.0;
            if (a_is_nonzero)
            {
                if (right)
                {
                    if (right->reference == 0)
                    {
                        if (sy_record_destroy(right) < 0)
                        {
                            return ERROR;
                        }
                    }
                }
                return left;
            }
            else
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }
                return right;
            }
        }
        else
        if (left->kind == RECORD_KIND_FLOAT64)
        {
            int a_is_nonzero = (*(double *)(left->value)) != 0.0;
            if (a_is_nonzero)
            {
                if (right)
                {
                    if (right->reference == 0)
                    {
                        if (sy_record_destroy(right) < 0)
                        {
                            return ERROR;
                        }
                    }
                }
                return left;
            }
            else
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }
                return right;
            }
        }
        else
        if (left->kind == RECORD_KIND_BIGFLOAT)
        {
            int a_is_nonzero = mpf_cmp_ui(*(mpf_t *)(left->value), 0) != 0;
            if (a_is_nonzero)
            {
                if (right)
                {
                    if (right->reference == 0)
                    {
                        if (sy_record_destroy(right) < 0)
                        {
                            return ERROR;
                        }
                    }
                }
                return left;
            }
            else
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }
                return right;
            }
        }
        else
        if (left->kind == RECORD_KIND_CHAR)
        {
            int a_is_nonzero = (*(char *)(left->value)) != 0;
            if (a_is_nonzero)
            {
                if (right)
                {
                    if (right->reference == 0)
                    {
                        if (sy_record_destroy(right) < 0)
                        {
                            return ERROR;
                        }
                    }
                }
                return left;
            }
            else
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }
                return right;
            }
        }
        else
        if (left->kind == RECORD_KIND_STRING)
        {
            int a_is_nonzero = strcmp((char *)(left->value), "") != 0;
            if (a_is_nonzero)
            {
                if (right)
                {
                    if (right->reference == 0)
                    {
                        if (sy_record_destroy(right) < 0)
                        {
                            return ERROR;
                        }
                    }
                }
                return left;
            }
            else
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }
                return right;
            }
        }
        else
        if (left->kind == RECORD_KIND_OBJECT)
        {
            if (right)
            {
                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }
            }
            return left;
        }
        else
        if (left->kind == RECORD_KIND_TUPLE)
        {
            if (right)
            {
                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }
            }
            return left;
        }
        else
        if (left->kind == RECORD_KIND_TYPE)
        {
            if (right)
            {
                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }
            }
            return left;
        }
        else
        if (left->kind == RECORD_KIND_STRUCT)
        {
            if (right)
            {
                if (right->reference == 0)
                {
                    if (sy_record_destroy(right) < 0)
                    {
                        return ERROR;
                    }
                }
            }
            return left;
        }
        else
        if (left->kind == RECORD_KIND_NULL)
        {
            int a_is_nonzero = (*(int64_t *)(left->value)) != 0;
            if (a_is_nonzero)
            {
                if (right)
                {
                    if (right->reference == 0)
                    {
                        if (sy_record_destroy(right) < 0)
                        {
                            return ERROR;
                        }
                    }
                }
                return left;
            }
            else
            {
                if (left->reference == 0)
                {
                    if (sy_record_destroy(left) < 0)
                    {
                        return ERROR;
                    }
                }
                return right;
            }
        }

        return NULL;
    }
    else
    {
        return sy_execute_logical_and(node, strip);
    }
}