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
#include <ffi.h>

#include "../types/types.h"
#include "../container/queue.h"
#include "../token/position.h"
#include "../token/token.h"
#include "../ast/node.h"
#include "../utils/utils.h"
#include "../utils/path.h"
#include "../error.h"
#include "../mutex.h"
#include "../memory.h"
#include "../config.h"
#include "../scanner/scanner.h"
#include "../parser/syntax/syntax.h"
#include "record.h"
#include "../repository.h"
#include "../interpreter.h"
#include "../thread.h"
#include "symbol_table.h"
#include "strip.h"
#include "entry.h"
#include "helper.h"
#include "execute.h"
int32_t
not_helper_id_cmp(not_node_t *n1, not_node_t *n2)
{
    not_node_basic_t *nb1 = (not_node_basic_t *)n1->value;
    not_node_basic_t *nb2 = (not_node_basic_t *)n2->value;

    return strcmp(nb1->value, nb2->value);
}

int32_t
not_helper_id_strcmp(not_node_t *n1, const char *name)
{
    not_node_basic_t *nb1 = (not_node_basic_t *)n1->value;

    return strcmp(nb1->value, name);
}