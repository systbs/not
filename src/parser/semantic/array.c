#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#include "../../types/types.h"
#include "../../container/list.h"
#include "../../container/stack.h"
#include "../../token/position.h"
#include "../../token/token.h"
#include "../../program.h"
#include "../../scanner/scanner.h"
#include "../../ast/node.h"
#include "../../utils/utils.h"
#include "../../utils/path.h"
#include "../syntax/syntax.h"
#include "../error.h"
#include "semantic.h"

int32_t
semantic_array(program_t *program, node_t *base, node_t *node, list_t *response, uint64_t flag)
{
    node_carrier_t *carrier1 = (node_carrier_t *)node->value;
    if (carrier1->data != NULL)
    {
        semantic_error(program, node, "Typing:it is wrong to use slice/split in typing\n\tInternal:%s-%u", 
            __FILE__, __LINE__);
        return -1;
    }
    
    ilist_t *il1 = list_rpush(response, node);
    if (il1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }
    return 1;
}
