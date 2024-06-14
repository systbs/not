#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#include <gmp.h>
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
sy_execute_postfix(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    if (node->kind == NODE_KIND_CALL)
    {
        return sy_call(node, strip, applicant, origin);
    }
    else if (node->kind == NODE_KIND_ARRAY)
    {
        return sy_execute_array(node, strip, applicant, origin);
    }
    else if (node->kind == NODE_KIND_ATTRIBUTE)
    {
        return sy_execute_attribute(node, strip, applicant, origin);
    }
    else if (node->kind == NODE_KIND_PSEUDONYM)
    {
        return sy_execute_pseudonym(node, strip, applicant, origin);
    }
    else
    {
        return sy_execute_primary(node, strip, applicant, origin);
    }
}
