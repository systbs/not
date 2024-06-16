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

not_record_t *
not_execute_instanceof(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin)
{
    if (node->kind == NODE_KIND_INSTANCEOF)
    {
        not_node_binary_t *binary = (not_node_binary_t *)node->value;
        not_record_t *left = not_execute_logical_or(binary->left, strip, applicant, origin);
        if (left == ERROR)
        {
            return ERROR;
        }

        not_record_t *right = not_execute_logical_or(binary->right, strip, applicant, origin);
        if (right == ERROR)
        {
            return ERROR;
        }

        int32_t r = 0;
        if (right->kind == RECORD_KIND_TYPE)
        {
            r = not_execute_value_check_by_type(node, left, right, strip, applicant);
            if (r < 0)
            {
                if (not_record_link_decrease(left) < 0)
                {
                    return ERROR;
                }

                if (not_record_link_decrease(right) < 0)
                {
                    return ERROR;
                }
                return ERROR;
            }
        }

        not_record_t *record = not_record_make_int_from_si(r);

        if (not_record_link_decrease(left) < 0)
        {
            return ERROR;
        }

        if (not_record_link_decrease(right) < 0)
        {
            return ERROR;
        }

        return record;
    }
    else
    {
        return not_execute_logical_or(node, strip, applicant, origin);
    }
}