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
sy_execute_conditional(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    if (node->kind == NODE_KIND_CONDITIONAL)
    {
        sy_node_triple_t *triple = (sy_node_triple_t *)node->value;

        sy_record_t *condition = sy_execute_instanceof(triple->base, strip, applicant, origin);
        if (condition == ERROR)
        {
            return ERROR;
        }

        int32_t truthy = sy_execute_truthy(condition);
        if (sy_record_link_decrease(condition) < 0)
        {
            return ERROR;
        }

        if (truthy)
        {
            return sy_execute_conditional(triple->left, strip, applicant, origin);
        }
        else
        {
            return sy_execute_conditional(triple->right, strip, applicant, origin);
        }
    }
    else
    {
        return sy_execute_instanceof(node, strip, applicant, origin);
    }
}