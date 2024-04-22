#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#include "../../types/types.h"
#include "../../container/list.h"
#include "../../token/position.h"
#include "../../token/token.h"
#include "../../program.h"
#include "../../scanner/scanner.h"
#include "../../ast/node.h"
#include "../../utils/utils.h"
#include "../../utils/path.h"
#include "../syntax.h"
#include "../error.h"
#include "../semantic.h"

static int32_t
semantic_id(program_t *program, node_t *base, node_t *node, list_t *response, uint64_t flag)
{
    //node_basic_t *basic1 = (node_basic_t *)node->value;
    //printf("semantic %s\n", basic1->value);
	return semantic_select(program, base, node, response, NULL, 0);
}

static int32_t
semantic_number(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    return 1;
}

static int32_t
semantic_char(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    return 1;
}

static int32_t
semantic_string(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    return 1;
}

static int32_t
semantic_null(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    return 1;
}

static int32_t
semantic_array(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    return 1;
}

static int32_t
semantic_dictionary(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    return 1;
}

static int32_t
semantic_lambda(program_t *program, node_t *node, list_t *response, uint64_t flag)
{
    ilist_t *il1 = list_rpush(response, node);
    if (il1 == NULL)
    {
        fprintf(stderr, "Internal:%s-%u\n\tUnable to allocate memory\n", __FILE__, __LINE__);
        return -1;
    }
    return 1;
}

int32_t
semantic_primary(program_t *program, node_t *base, node_t *node, list_t *response, uint64_t flag)
{
    if (node->kind == NODE_KIND_ID)
    {
        return semantic_id(program, base, node, response, flag);
    }
    else
    if (node->kind == NODE_KIND_NUMBER)
    {
        return semantic_number(program, node, response, flag);
    }
    else
    if (node->kind == NODE_KIND_CHAR)
    {
        return semantic_char(program, node, response, flag);
    }
    else
    if (node->kind == NODE_KIND_STRING)
    {
        return semantic_string(program, node, response, flag);
    }
    else
    if (node->kind == NODE_KIND_NULL)
    {
        return semantic_null(program, node, response, flag);
    }
    else
    if (node->kind == NODE_KIND_ARRAY)
    {
        return semantic_array(program, node, response, flag);
    }
    else
    if (node->kind == NODE_KIND_DICTIONARY)
    {
        return semantic_dictionary(program, node, response, flag);
    }
    else
    if (node->kind == NODE_KIND_LAMBDA)
    {
        return semantic_lambda(program, node, response, flag);
    }
    else
    {
        semantic_error(program, node, "unselectable\n\tInternal:%s-%u", __FILE__, __LINE__);
        return -1;
    }
}
