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
#include "../syntax/syntax.h"
#include "../error.h"
#include "semantic.h"

node_t *
semantic_make_field(program_t *program, node_t *arg)
{
	node_t *node = node_create(arg->parent, arg->position);
	if (node == NULL)
	{
		return NULL;
	}
    node->flag = NODE_FLAG_TEMPORARY;

	return node_make_field(node, arg, NULL);
}

node_t *
semantic_make_fields(program_t *program, node_t *parent, uint64_t n, ...)
{
	node_t *node = node_create(parent, parent->position);
	if (node == NULL)
	{
		return NULL;
	}
    node->flag = NODE_FLAG_TEMPORARY;

	list_t *fields = list_create();
	if (fields == NULL)
	{
		return NULL;
	}

    va_list arg;
    va_start(arg, n);
 
    for (uint64_t i = 0; i < n; i++)
    {
        node_t *node2 = semantic_make_field(program, (node_t *)va_arg(arg, node_t *));
		if (node2 == NULL)
		{
			return NULL;
		}

		if (!list_rpush(fields, node2))
		{
			return NULL;
		}
    }
    
    va_end(arg);

	return node_make_fields(node, fields);
}

node_t *
semantic_make_argument(program_t *program, node_t *arg)
{
	node_t *node = node_create(arg->parent, arg->position);
	if (node == NULL)
	{
		return NULL;
	}
    node->flag = NODE_FLAG_TEMPORARY;

	return node_make_argument(node, arg, NULL);
}

node_t *
semantic_make_arguments(program_t *program, node_t *parent, uint64_t n, ...)
{
	node_t *node = node_create(parent, parent->position);
	if (node == NULL)
	{
		return NULL;
	}
    node->flag = NODE_FLAG_TEMPORARY;

	list_t *arguments = list_create();
	if (arguments == NULL)
	{
		return NULL;
	}

    va_list arg;
    va_start(arg, n);
 
    for (uint64_t i = 0; i < n; i++)
    {
        node_t *node2 = semantic_make_argument(program, (node_t *)va_arg(arg, node_t *));
		if (node2 == NULL)
		{
			return NULL;
		}

		if (!list_rpush(arguments, node2))
		{
			return NULL;
		}
    }
    
    va_end(arg);

	return node_make_arguments(node, arguments);
}

