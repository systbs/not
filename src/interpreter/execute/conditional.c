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

#include "../symbol_table.h"
#include "../strip.h"
#include "execute.h"

not_record_t *
not_conditional(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin)
{
    if (node->kind == NODE_KIND_CONDITIONAL)
    {
        not_node_triple_t *triple = (not_node_triple_t *)node->value;

        not_record_t *condition = not_instanceof(triple->base, strip, applicant, origin);
        if (condition == NOT_PTR_ERROR)
        {
            return NOT_PTR_ERROR;
        }

        int32_t truthy = not_execute_truthy(condition);
        if (not_record_link_decrease(condition) < 0)
        {
            return NOT_PTR_ERROR;
        }

        if (truthy)
        {
            return not_conditional(triple->left, strip, applicant, origin);
        }
        else
        {
            return not_conditional(triple->right, strip, applicant, origin);
        }
    }
    else
    {
        return not_instanceof(node, strip, applicant, origin);
    }
}