/*
 * Creator: Yasser Sajjadi(Ys)
 * Date: 05/04/2024
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#include "../types/types.h"
#include "../container/list.h"
#include "../token/position.h"
#include "../token/token.h"
#include "../program.h"
#include "../scanner/scanner.h"
#include "../ast/node.h"
#include "../utils/utils.h"
#include "../utils/path.h"
#include "syntax.h"
#include "error.h"

uint64_t
mensuration_size(program_t *program, node_t *node)
{
    return 5;
}