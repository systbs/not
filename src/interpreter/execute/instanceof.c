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
sy_execute_instanceof(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    if (node->kind == NODE_KIND_INSTANCEOF)
    {
        sy_node_binary_t *binary = (sy_node_binary_t *)node->value;
        sy_record_t *left = sy_execute_logical_or(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return ERROR;
        }

        sy_record_t *right = sy_execute_logical_or(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            return ERROR;
        }

        int32_t r = 0;
        if (right->kind == RECORD_KIND_TYPE)
        {
            r = sy_execute_value_check_by_type(node, left, right, strip, applicant);
            if (r < 0)
            {
                if (sy_record_link_decrease(left) < 0)
                {
                    return ERROR;
                }

                if (sy_record_link_decrease(right) < 0)
                {
                    return ERROR;
                }
                return ERROR;
            }
        }

        sy_record_t *record = sy_record_make_int_from_si(r);

        if (sy_record_link_decrease(left) < 0)
        {
            return ERROR;
        }

        if (sy_record_link_decrease(right) < 0)
        {
            return ERROR;
        }

        return record;
    }
    else
    {
        return sy_execute_logical_or(node, strip, applicant, origin);
    }
}