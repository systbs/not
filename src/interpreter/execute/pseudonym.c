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
sy_execute_pseudonym(sy_node_t *node, sy_strip_t *strip, sy_node_t *applicant, sy_node_t *origin)
{
    sy_node_carrier_t *carrier = (sy_node_carrier_t *)node->value;

    sy_record_t *base = sy_execute_expression(carrier->base, strip, applicant, NULL);
    if (base == ERROR)
    {
        return ERROR;
    }

    if (base->kind != RECORD_KIND_TYPE)
    {
        goto region_no_genericable;
    }
    
    sy_record_type_t *base_type = (sy_record_type_t *)base->value;
    if (base_type->type->kind == NODE_KIND_CLASS)
    {

    }
    else
    if (base_type->type->kind == NODE_KIND_FUN)
    {
        
    }
    else
    if (base_type->type->kind == NODE_KIND_LAMBDA)
    {
        
    }
    else
    {
        goto region_no_genericable;
    }
    
    return base;

    region_no_genericable:
    sy_error_type_by_node(node, "it has no generic type");
    return ERROR;
}