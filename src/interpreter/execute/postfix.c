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

#include "../symbol_table.h"
#include "../strip.h"
#include "execute.h"

not_record_t *
not_postfix(not_node_t *node, not_strip_t *strip, not_node_t *applicant, not_node_t *origin)
{
    if (node->kind == NODE_KIND_CALL)
    {
        return not_call(node, strip, applicant, origin);
    }
    else if (node->kind == NODE_KIND_ARRAY)
    {
        return not_array(node, strip, applicant, origin);
    }
    else if (node->kind == NODE_KIND_ATTRIBUTE)
    {
        return not_attribute(node, strip, applicant, origin);
    }
    else if (node->kind == NODE_KIND_PSEUDONYM)
    {
        return not_pseudonym(node, strip, applicant, origin);
    }
    else
    {
        return not_primary(node, strip, applicant, origin);
    }
}
