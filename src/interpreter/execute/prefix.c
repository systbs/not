#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#include <gmp.h>
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
sy_execute_prefix(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    if (node->kind == NODE_KIND_TILDE)
    {
        sy_node_unary_t *unary = (sy_node_unary_t *)node->value;

        sy_record_t *left = sy_record_make_int8(-1);
        if (left == ERROR)
        {
            return ERROR;
        }

        sy_record_t *right = sy_execute_prefix(unary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            left->link -= 1;
            return ERROR;
        }

        sy_record_t *record = sy_execute_xor(node, left, right, applicant);

        left->link -= 1;
        right->link -= 1;

        return record;
    }
    else
    if (node->kind == NODE_KIND_POS)
    {
        sy_node_unary_t *unary = (sy_node_unary_t *)node->value;

        sy_record_t *left = sy_record_make_int8(0);
        if (left == ERROR)
        {
            return ERROR;
        }

        sy_record_t *right = sy_execute_prefix(unary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            left->link -= 1;
            return ERROR;
        }

        sy_record_t *record = sy_execute_plus(node, left, right, applicant);

        left->link -= 1;
        right->link -= 1;

        return record;
    }
    else
    if (node->kind == NODE_KIND_NEG)
    {
        sy_node_unary_t *unary = (sy_node_unary_t *)node->value;

        sy_record_t *left = sy_record_make_int8(0);
        if (left == ERROR)
        {
            return ERROR;
        }

        sy_record_t *right = sy_execute_prefix(unary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            left->link -= 1;
            return ERROR;
        }

        sy_record_t *record = sy_execute_minus(node, left, right, applicant);

        left->link -= 1;
        right->link -= 1;

        return record;
    }
    else
    if (node->kind == NODE_KIND_NOT)
    {
        sy_node_unary_t *unary = (sy_node_unary_t *)node->value;

        sy_record_t *left = sy_record_make_int8(0);
        if (left == ERROR)
        {
            return ERROR;
        }

        sy_record_t *right = sy_execute_prefix(unary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            left->link -= 1;
            return ERROR;
        }

        sy_record_t *record = sy_execute_eq(node, left, right, applicant);

        left->link -= 1;
        right->link -= 1;

        return record;
    }
    else
    {
        return sy_execute_postfix(node, strip, applicant, origin);
    }
}

