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
semantic_call(program_t *program, node_t *base, node_t *node, list_t *response, uint64_t flag)
{
    semantic_error(program, node, "Typing:it is wrong to use call in typing\n\tInternal:%s-%u", 
        __FILE__, __LINE__);
    return -1;
}
