#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../types/types.h"
#include "../container/list.h"
#include "../token/position.h"
#include "../token/token.h"
#include "../program.h"
#include "../scanner/scanner.h"
#include "node.h"
#include "../parser/syntax.h"

uint64_t node_counter = 0;

void
node_destroy(node_t *node)
{
  free (node);
}

node_t *
node_create(node_t *parent, position_t position)
{
	node_t *node = (node_t *)malloc(sizeof(node_t));
	if(node == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
		return NULL;
	}
	memset(node, 0, sizeof(node_t));

	node->id = node_counter++;
	node->position = position;
	node->parent = parent;
	node->flag = NODE_FLAG_NONE;

	return node;
}

static void
node_update(node_t *node, int32_t kind, void *value)
{
	node->value = value;
	node->kind = kind;
}

node_t *
node_clone(node_t *parent, node_t *node)
{
	if (node->kind == NODE_KIND_ID)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_basic_t *basic1 = (node_basic_t *)node->value;

		node_basic_t *basic2 = (node_basic_t *)malloc(sizeof(node_basic_t));
		if(basic2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_basic_t));
			return NULL;
		}
		memset(basic2, 0, sizeof(node_basic_t));
		
		basic2->value = basic1->value;

		node1->value = basic2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_NUMBER)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_basic_t *basic1 = (node_basic_t *)node->value;

		node_basic_t *basic2 = (node_basic_t *)malloc(sizeof(node_basic_t));
		if(basic2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_basic_t));
			return NULL;
		}
		memset(basic2, 0, sizeof(node_basic_t));
		
		basic2->value = basic1->value;

		node1->value = basic2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_CHAR)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_basic_t *basic1 = (node_basic_t *)node->value;

		node_basic_t *basic2 = (node_basic_t *)malloc(sizeof(node_basic_t));
		if(basic2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_basic_t));
			return NULL;
		}
		memset(basic2, 0, sizeof(node_basic_t));
		
		basic2->value = basic1->value;

		node1->value = basic2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_STRING)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_basic_t *basic1 = (node_basic_t *)node->value;

		node_basic_t *basic2 = (node_basic_t *)malloc(sizeof(node_basic_t));
		if(basic2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_basic_t));
			return NULL;
		}
		memset(basic2, 0, sizeof(node_basic_t));
		
		basic2->value = basic1->value;

		node1->value = basic2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_NULL)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_basic_t *basic1 = (node_basic_t *)node->value;

		node_basic_t *basic2 = (node_basic_t *)malloc(sizeof(node_basic_t));
		if(basic2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_basic_t));
			return NULL;
		}
		memset(basic2, 0, sizeof(node_basic_t));
		
		basic2->value = basic1->value;

		node1->value = basic2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_THIS)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_basic_t *basic1 = (node_basic_t *)node->value;

		node_basic_t *basic2 = (node_basic_t *)malloc(sizeof(node_basic_t));
		if(basic2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_basic_t));
			return NULL;
		}
		memset(basic2, 0, sizeof(node_basic_t));
		
		basic2->value = basic1->value;

		node1->value = basic2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_ARRAY)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_block_t *block1 = (node_block_t *)node->value;

		node_block_t *block2 = (node_block_t *)malloc(sizeof(node_block_t));
		if(block2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
			return NULL;
		}
		memset(block2, 0, sizeof(node_block_t));
		
		list_t *list1 = list_create();
		if (list1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(list_t));
			return NULL;
		}

		ilist_t *a1;
		for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
		{
			node_t *item1 = (node_t *)a1->value;
			node_t *item2 = node_clone(node1, item1);
			if (item2 == NULL)
			{
				return NULL;
			}

			ilist_t *r1 = list_rpush(list1, item2);
			if (r1 == NULL)
			{
				return NULL;
			}
		}

		block2->list = list1;

		node1->value = block2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_DICTIONARY)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_block_t *block1 = (node_block_t *)node->value;

		node_block_t *block2 = (node_block_t *)malloc(sizeof(node_block_t));
		if(block2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
			return NULL;
		}
		memset(block2, 0, sizeof(node_block_t));
		
		list_t *list1 = list_create();
		if (list1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(list_t));
			return NULL;
		}

		ilist_t *a1;
		for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
		{
			node_t *item1 = (node_t *)a1->value;
			node_t *item2 = node_clone(node1, item1);
			if (item2 == NULL)
			{
				return NULL;
			}

			ilist_t *r1 = list_rpush(list1, item2);
			if (r1 == NULL)
			{
				return NULL;
			}
		}

		block2->list = list1;

		node1->value = block2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_PSEUDONYM)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_carrier_t *carrier1 = (node_carrier_t *)node->value;

		node_carrier_t *carrier2 = (node_carrier_t *)malloc(sizeof(node_carrier_t));
		if(carrier2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_carrier_t));
			return NULL;
		}
		memset(carrier2, 0, sizeof(node_carrier_t));

		if (carrier1->base != NULL)
		{
			carrier2->base = node_clone(node1, carrier1->base);
			if (carrier2->base == NULL)
			{
				return NULL;
			}
		}

		if (carrier1->data != NULL)
		{
			carrier2->data = node_clone(node1, carrier1->data);
			if (carrier2->data == NULL)
			{
				return NULL;
			}
		}

		node1->value = carrier2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_SIZEOF)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_unary_t *unary1 = (node_unary_t *)node->value;

		node_unary_t *unary2 = (node_unary_t *)malloc(sizeof(node_unary_t));
		if(unary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
			return NULL;
		}
		memset(unary2, 0, sizeof(node_unary_t));

		unary2->right = node_clone(node1, unary1->right);
		if (unary2->right == NULL)
		{
			return NULL;
		}

		node1->value = unary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_TYPEOF)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_unary_t *unary1 = (node_unary_t *)node->value;

		node_unary_t *unary2 = (node_unary_t *)malloc(sizeof(node_unary_t));
		if(unary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
			return NULL;
		}
		memset(unary2, 0, sizeof(node_unary_t));

		unary2->right = node_clone(node1, unary1->right);
		if (unary2->right == NULL)
		{
			return NULL;
		}

		node1->value = unary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_PARENTHESIS)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_unary_t *unary1 = (node_unary_t *)node->value;

		node_unary_t *unary2 = (node_unary_t *)malloc(sizeof(node_unary_t));
		if(unary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
			return NULL;
		}
		memset(unary2, 0, sizeof(node_unary_t));

		unary2->right = node_clone(node1, unary1->right);
		if (unary2->right == NULL)
		{
			return NULL;
		}

		node1->value = unary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_CALL)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_carrier_t *carrier1 = (node_carrier_t *)node->value;

		node_carrier_t *carrier2 = (node_carrier_t *)malloc(sizeof(node_carrier_t));
		if(carrier2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_carrier_t));
			return NULL;
		}
		memset(carrier2, 0, sizeof(node_carrier_t));

		if (carrier1->base != NULL)
		{
			carrier2->base = node_clone(node1, carrier1->base);
			if (carrier2->base == NULL)
			{
				return NULL;
			}
		}

		if (carrier1->data != NULL)
		{
			carrier2->data = node_clone(node1, carrier1->data);
			if (carrier2->data == NULL)
			{
				return NULL;
			}
		}

		node1->value = carrier2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_ITEM)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_carrier_t *carrier1 = (node_carrier_t *)node->value;

		node_carrier_t *carrier2 = (node_carrier_t *)malloc(sizeof(node_carrier_t));
		if(carrier2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_carrier_t));
			return NULL;
		}
		memset(carrier2, 0, sizeof(node_carrier_t));

		if (carrier1->base != NULL)
		{
			carrier2->base = node_clone(node1, carrier1->base);
			if (carrier2->base == NULL)
			{
				return NULL;
			}
		}

		if (carrier1->data != NULL)
		{
			carrier2->data = node_clone(node1, carrier1->data);
			if (carrier2->data == NULL)
			{
				return NULL;
			}
		}

		node1->value = carrier2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_ATTRIBUTE)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_binary_t *binary1 = (node_binary_t *)node->value;

		node_binary_t *binary2 = (node_binary_t *)malloc(sizeof(node_binary_t));
		if(binary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
			return NULL;
		}
		memset(binary2, 0, sizeof(node_binary_t));

		binary2->left = node_clone(node1, binary1->left);
		if (binary2->left == NULL)
		{
			return NULL;
		}

		binary2->right = node_clone(node1, binary1->right);
		if (binary2->right == NULL)
		{
			return NULL;
		}

		node1->value = binary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_TILDE)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_unary_t *unary1 = (node_unary_t *)node->value;

		node_unary_t *unary2 = (node_unary_t *)malloc(sizeof(node_unary_t));
		if(unary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
			return NULL;
		}
		memset(unary2, 0, sizeof(node_unary_t));

		unary2->right = node_clone(node1, unary1->right);
		if (unary2->right == NULL)
		{
			return NULL;
		}

		node1->value = unary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_NOT)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_unary_t *unary1 = (node_unary_t *)node->value;

		node_unary_t *unary2 = (node_unary_t *)malloc(sizeof(node_unary_t));
		if(unary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
			return NULL;
		}
		memset(unary2, 0, sizeof(node_unary_t));

		unary2->right = node_clone(node1, unary1->right);
		if (unary2->right == NULL)
		{
			return NULL;
		}

		node1->value = unary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_NEG)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_unary_t *unary1 = (node_unary_t *)node->value;

		node_unary_t *unary2 = (node_unary_t *)malloc(sizeof(node_unary_t));
		if(unary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
			return NULL;
		}
		memset(unary2, 0, sizeof(node_unary_t));

		unary2->right = node_clone(node1, unary1->right);
		if (unary2->right == NULL)
		{
			return NULL;
		}

		node1->value = unary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_POS)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_unary_t *unary1 = (node_unary_t *)node->value;

		node_unary_t *unary2 = (node_unary_t *)malloc(sizeof(node_unary_t));
		if(unary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
			return NULL;
		}
		memset(unary2, 0, sizeof(node_unary_t));

		unary2->right = node_clone(node1, unary1->right);
		if (unary2->right == NULL)
		{
			return NULL;
		}

		node1->value = unary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_POW)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_binary_t *binary1 = (node_binary_t *)node->value;

		node_binary_t *binary2 = (node_binary_t *)malloc(sizeof(node_binary_t));
		if(binary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
			return NULL;
		}
		memset(binary2, 0, sizeof(node_binary_t));

		binary2->left = node_clone(node1, binary1->left);
		if (binary2->left == NULL)
		{
			return NULL;
		}

		binary2->right = node_clone(node1, binary1->right);
		if (binary2->right == NULL)
		{
			return NULL;
		}

		node1->value = binary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_EPI)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_binary_t *binary1 = (node_binary_t *)node->value;

		node_binary_t *binary2 = (node_binary_t *)malloc(sizeof(node_binary_t));
		if(binary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
			return NULL;
		}
		memset(binary2, 0, sizeof(node_binary_t));

		binary2->left = node_clone(node1, binary1->left);
		if (binary2->left == NULL)
		{
			return NULL;
		}

		binary2->right = node_clone(node1, binary1->right);
		if (binary2->right == NULL)
		{
			return NULL;
		}

		node1->value = binary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_MUL)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_binary_t *binary1 = (node_binary_t *)node->value;

		node_binary_t *binary2 = (node_binary_t *)malloc(sizeof(node_binary_t));
		if(binary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
			return NULL;
		}
		memset(binary2, 0, sizeof(node_binary_t));

		binary2->left = node_clone(node1, binary1->left);
		if (binary2->left == NULL)
		{
			return NULL;
		}

		binary2->right = node_clone(node1, binary1->right);
		if (binary2->right == NULL)
		{
			return NULL;
		}

		node1->value = binary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_DIV)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_binary_t *binary1 = (node_binary_t *)node->value;

		node_binary_t *binary2 = (node_binary_t *)malloc(sizeof(node_binary_t));
		if(binary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
			return NULL;
		}
		memset(binary2, 0, sizeof(node_binary_t));

		binary2->left = node_clone(node1, binary1->left);
		if (binary2->left == NULL)
		{
			return NULL;
		}

		binary2->right = node_clone(node1, binary1->right);
		if (binary2->right == NULL)
		{
			return NULL;
		}

		node1->value = binary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_MOD)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_binary_t *binary1 = (node_binary_t *)node->value;

		node_binary_t *binary2 = (node_binary_t *)malloc(sizeof(node_binary_t));
		if(binary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
			return NULL;
		}
		memset(binary2, 0, sizeof(node_binary_t));

		binary2->left = node_clone(node1, binary1->left);
		if (binary2->left == NULL)
		{
			return NULL;
		}

		binary2->right = node_clone(node1, binary1->right);
		if (binary2->right == NULL)
		{
			return NULL;
		}

		node1->value = binary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_PLUS)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_binary_t *binary1 = (node_binary_t *)node->value;

		node_binary_t *binary2 = (node_binary_t *)malloc(sizeof(node_binary_t));
		if(binary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
			return NULL;
		}
		memset(binary2, 0, sizeof(node_binary_t));

		binary2->left = node_clone(node1, binary1->left);
		if (binary2->left == NULL)
		{
			return NULL;
		}

		binary2->right = node_clone(node1, binary1->right);
		if (binary2->right == NULL)
		{
			return NULL;
		}

		node1->value = binary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_MINUS)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_binary_t *binary1 = (node_binary_t *)node->value;

		node_binary_t *binary2 = (node_binary_t *)malloc(sizeof(node_binary_t));
		if(binary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
			return NULL;
		}
		memset(binary2, 0, sizeof(node_binary_t));

		binary2->left = node_clone(node1, binary1->left);
		if (binary2->left == NULL)
		{
			return NULL;
		}

		binary2->right = node_clone(node1, binary1->right);
		if (binary2->right == NULL)
		{
			return NULL;
		}

		node1->value = binary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_SHL)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_binary_t *binary1 = (node_binary_t *)node->value;

		node_binary_t *binary2 = (node_binary_t *)malloc(sizeof(node_binary_t));
		if(binary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
			return NULL;
		}
		memset(binary2, 0, sizeof(node_binary_t));

		binary2->left = node_clone(node1, binary1->left);
		if (binary2->left == NULL)
		{
			return NULL;
		}

		binary2->right = node_clone(node1, binary1->right);
		if (binary2->right == NULL)
		{
			return NULL;
		}

		node1->value = binary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_SHR)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_binary_t *binary1 = (node_binary_t *)node->value;

		node_binary_t *binary2 = (node_binary_t *)malloc(sizeof(node_binary_t));
		if(binary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
			return NULL;
		}
		memset(binary2, 0, sizeof(node_binary_t));

		binary2->left = node_clone(node1, binary1->left);
		if (binary2->left == NULL)
		{
			return NULL;
		}

		binary2->right = node_clone(node1, binary1->right);
		if (binary2->right == NULL)
		{
			return NULL;
		}

		node1->value = binary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_LT)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_binary_t *binary1 = (node_binary_t *)node->value;

		node_binary_t *binary2 = (node_binary_t *)malloc(sizeof(node_binary_t));
		if(binary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
			return NULL;
		}
		memset(binary2, 0, sizeof(node_binary_t));

		binary2->left = node_clone(node1, binary1->left);
		if (binary2->left == NULL)
		{
			return NULL;
		}

		binary2->right = node_clone(node1, binary1->right);
		if (binary2->right == NULL)
		{
			return NULL;
		}

		node1->value = binary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_LE)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_binary_t *binary1 = (node_binary_t *)node->value;

		node_binary_t *binary2 = (node_binary_t *)malloc(sizeof(node_binary_t));
		if(binary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
			return NULL;
		}
		memset(binary2, 0, sizeof(node_binary_t));

		binary2->left = node_clone(node1, binary1->left);
		if (binary2->left == NULL)
		{
			return NULL;
		}

		binary2->right = node_clone(node1, binary1->right);
		if (binary2->right == NULL)
		{
			return NULL;
		}

		node1->value = binary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_GT)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_binary_t *binary1 = (node_binary_t *)node->value;

		node_binary_t *binary2 = (node_binary_t *)malloc(sizeof(node_binary_t));
		if(binary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
			return NULL;
		}
		memset(binary2, 0, sizeof(node_binary_t));

		binary2->left = node_clone(node1, binary1->left);
		if (binary2->left == NULL)
		{
			return NULL;
		}

		binary2->right = node_clone(node1, binary1->right);
		if (binary2->right == NULL)
		{
			return NULL;
		}

		node1->value = binary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_GE)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_binary_t *binary1 = (node_binary_t *)node->value;

		node_binary_t *binary2 = (node_binary_t *)malloc(sizeof(node_binary_t));
		if(binary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
			return NULL;
		}
		memset(binary2, 0, sizeof(node_binary_t));

		binary2->left = node_clone(node1, binary1->left);
		if (binary2->left == NULL)
		{
			return NULL;
		}

		binary2->right = node_clone(node1, binary1->right);
		if (binary2->right == NULL)
		{
			return NULL;
		}

		node1->value = binary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_EQ)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_binary_t *binary1 = (node_binary_t *)node->value;

		node_binary_t *binary2 = (node_binary_t *)malloc(sizeof(node_binary_t));
		if(binary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
			return NULL;
		}
		memset(binary2, 0, sizeof(node_binary_t));

		binary2->left = node_clone(node1, binary1->left);
		if (binary2->left == NULL)
		{
			return NULL;
		}

		binary2->right = node_clone(node1, binary1->right);
		if (binary2->right == NULL)
		{
			return NULL;
		}

		node1->value = binary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_NEQ)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_binary_t *binary1 = (node_binary_t *)node->value;

		node_binary_t *binary2 = (node_binary_t *)malloc(sizeof(node_binary_t));
		if(binary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
			return NULL;
		}
		memset(binary2, 0, sizeof(node_binary_t));

		binary2->left = node_clone(node1, binary1->left);
		if (binary2->left == NULL)
		{
			return NULL;
		}

		binary2->right = node_clone(node1, binary1->right);
		if (binary2->right == NULL)
		{
			return NULL;
		}

		node1->value = binary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_AND)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_binary_t *binary1 = (node_binary_t *)node->value;

		node_binary_t *binary2 = (node_binary_t *)malloc(sizeof(node_binary_t));
		if(binary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
			return NULL;
		}
		memset(binary2, 0, sizeof(node_binary_t));

		binary2->left = node_clone(node1, binary1->left);
		if (binary2->left == NULL)
		{
			return NULL;
		}

		binary2->right = node_clone(node1, binary1->right);
		if (binary2->right == NULL)
		{
			return NULL;
		}

		node1->value = binary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_OR)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_binary_t *binary1 = (node_binary_t *)node->value;

		node_binary_t *binary2 = (node_binary_t *)malloc(sizeof(node_binary_t));
		if(binary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
			return NULL;
		}
		memset(binary2, 0, sizeof(node_binary_t));

		binary2->left = node_clone(node1, binary1->left);
		if (binary2->left == NULL)
		{
			return NULL;
		}

		binary2->right = node_clone(node1, binary1->right);
		if (binary2->right == NULL)
		{
			return NULL;
		}

		node1->value = binary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_XOR)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_binary_t *binary1 = (node_binary_t *)node->value;

		node_binary_t *binary2 = (node_binary_t *)malloc(sizeof(node_binary_t));
		if(binary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
			return NULL;
		}
		memset(binary2, 0, sizeof(node_binary_t));

		binary2->left = node_clone(node1, binary1->left);
		if (binary2->left == NULL)
		{
			return NULL;
		}

		binary2->right = node_clone(node1, binary1->right);
		if (binary2->right == NULL)
		{
			return NULL;
		}

		node1->value = binary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_LAND)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_binary_t *binary1 = (node_binary_t *)node->value;

		node_binary_t *binary2 = (node_binary_t *)malloc(sizeof(node_binary_t));
		if(binary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
			return NULL;
		}
		memset(binary2, 0, sizeof(node_binary_t));

		binary2->left = node_clone(node1, binary1->left);
		if (binary2->left == NULL)
		{
			return NULL;
		}

		binary2->right = node_clone(node1, binary1->right);
		if (binary2->right == NULL)
		{
			return NULL;
		}

		node1->value = binary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_LOR)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_binary_t *binary1 = (node_binary_t *)node->value;

		node_binary_t *binary2 = (node_binary_t *)malloc(sizeof(node_binary_t));
		if(binary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
			return NULL;
		}
		memset(binary2, 0, sizeof(node_binary_t));

		binary2->left = node_clone(node1, binary1->left);
		if (binary2->left == NULL)
		{
			return NULL;
		}

		binary2->right = node_clone(node1, binary1->right);
		if (binary2->right == NULL)
		{
			return NULL;
		}

		node1->value = binary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_ASSIGN)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_binary_t *binary1 = (node_binary_t *)node->value;

		node_binary_t *binary2 = (node_binary_t *)malloc(sizeof(node_binary_t));
		if(binary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
			return NULL;
		}
		memset(binary2, 0, sizeof(node_binary_t));

		binary2->left = node_clone(node1, binary1->left);
		if (binary2->left == NULL)
		{
			return NULL;
		}

		binary2->right = node_clone(node1, binary1->right);
		if (binary2->right == NULL)
		{
			return NULL;
		}

		node1->value = binary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_DEFINE)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_binary_t *binary1 = (node_binary_t *)node->value;

		node_binary_t *binary2 = (node_binary_t *)malloc(sizeof(node_binary_t));
		if(binary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
			return NULL;
		}
		memset(binary2, 0, sizeof(node_binary_t));

		binary2->left = node_clone(node1, binary1->left);
		if (binary2->left == NULL)
		{
			return NULL;
		}

		binary2->right = node_clone(node1, binary1->right);
		if (binary2->right == NULL)
		{
			return NULL;
		}

		node1->value = binary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_ADD_ASSIGN)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_binary_t *binary1 = (node_binary_t *)node->value;

		node_binary_t *binary2 = (node_binary_t *)malloc(sizeof(node_binary_t));
		if(binary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
			return NULL;
		}
		memset(binary2, 0, sizeof(node_binary_t));

		binary2->left = node_clone(node1, binary1->left);
		if (binary2->left == NULL)
		{
			return NULL;
		}

		binary2->right = node_clone(node1, binary1->right);
		if (binary2->right == NULL)
		{
			return NULL;
		}

		node1->value = binary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_SUB_ASSIGN)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_binary_t *binary1 = (node_binary_t *)node->value;

		node_binary_t *binary2 = (node_binary_t *)malloc(sizeof(node_binary_t));
		if(binary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
			return NULL;
		}
		memset(binary2, 0, sizeof(node_binary_t));

		binary2->left = node_clone(node1, binary1->left);
		if (binary2->left == NULL)
		{
			return NULL;
		}

		binary2->right = node_clone(node1, binary1->right);
		if (binary2->right == NULL)
		{
			return NULL;
		}

		node1->value = binary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_DIV_ASSIGN)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_binary_t *binary1 = (node_binary_t *)node->value;

		node_binary_t *binary2 = (node_binary_t *)malloc(sizeof(node_binary_t));
		if(binary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
			return NULL;
		}
		memset(binary2, 0, sizeof(node_binary_t));

		binary2->left = node_clone(node1, binary1->left);
		if (binary2->left == NULL)
		{
			return NULL;
		}

		binary2->right = node_clone(node1, binary1->right);
		if (binary2->right == NULL)
		{
			return NULL;
		}

		node1->value = binary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_MUL_ASSIGN)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_binary_t *binary1 = (node_binary_t *)node->value;

		node_binary_t *binary2 = (node_binary_t *)malloc(sizeof(node_binary_t));
		if(binary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
			return NULL;
		}
		memset(binary2, 0, sizeof(node_binary_t));

		binary2->left = node_clone(node1, binary1->left);
		if (binary2->left == NULL)
		{
			return NULL;
		}

		binary2->right = node_clone(node1, binary1->right);
		if (binary2->right == NULL)
		{
			return NULL;
		}

		node1->value = binary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_MOD_ASSIGN)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_binary_t *binary1 = (node_binary_t *)node->value;

		node_binary_t *binary2 = (node_binary_t *)malloc(sizeof(node_binary_t));
		if(binary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
			return NULL;
		}
		memset(binary2, 0, sizeof(node_binary_t));

		binary2->left = node_clone(node1, binary1->left);
		if (binary2->left == NULL)
		{
			return NULL;
		}

		binary2->right = node_clone(node1, binary1->right);
		if (binary2->right == NULL)
		{
			return NULL;
		}

		node1->value = binary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_AND_ASSIGN)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_binary_t *binary1 = (node_binary_t *)node->value;

		node_binary_t *binary2 = (node_binary_t *)malloc(sizeof(node_binary_t));
		if(binary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
			return NULL;
		}
		memset(binary2, 0, sizeof(node_binary_t));

		binary2->left = node_clone(node1, binary1->left);
		if (binary2->left == NULL)
		{
			return NULL;
		}

		binary2->right = node_clone(node1, binary1->right);
		if (binary2->right == NULL)
		{
			return NULL;
		}

		node1->value = binary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_OR_ASSIGN)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_binary_t *binary1 = (node_binary_t *)node->value;

		node_binary_t *binary2 = (node_binary_t *)malloc(sizeof(node_binary_t));
		if(binary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
			return NULL;
		}
		memset(binary2, 0, sizeof(node_binary_t));

		binary2->left = node_clone(node1, binary1->left);
		if (binary2->left == NULL)
		{
			return NULL;
		}

		binary2->right = node_clone(node1, binary1->right);
		if (binary2->right == NULL)
		{
			return NULL;
		}

		node1->value = binary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_SHL_ASSIGN)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_binary_t *binary1 = (node_binary_t *)node->value;

		node_binary_t *binary2 = (node_binary_t *)malloc(sizeof(node_binary_t));
		if(binary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
			return NULL;
		}
		memset(binary2, 0, sizeof(node_binary_t));

		binary2->left = node_clone(node1, binary1->left);
		if (binary2->left == NULL)
		{
			return NULL;
		}

		binary2->right = node_clone(node1, binary1->right);
		if (binary2->right == NULL)
		{
			return NULL;
		}

		node1->value = binary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_SHR_ASSIGN)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_binary_t *binary1 = (node_binary_t *)node->value;

		node_binary_t *binary2 = (node_binary_t *)malloc(sizeof(node_binary_t));
		if(binary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
			return NULL;
		}
		memset(binary2, 0, sizeof(node_binary_t));

		binary2->left = node_clone(node1, binary1->left);
		if (binary2->left == NULL)
		{
			return NULL;
		}

		binary2->right = node_clone(node1, binary1->right);
		if (binary2->right == NULL)
		{
			return NULL;
		}

		node1->value = binary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_BREAK)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_unary_t *unary1 = (node_unary_t *)node->value;

		node_unary_t *unary2 = (node_unary_t *)malloc(sizeof(node_unary_t));
		if(unary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
			return NULL;
		}
		memset(unary2, 0, sizeof(node_unary_t));

		unary2->right = node_clone(node1, unary1->right);
		if (unary2->right == NULL)
		{
			return NULL;
		}

		node1->value = unary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_CONTINUE)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_unary_t *unary1 = (node_unary_t *)node->value;

		node_unary_t *unary2 = (node_unary_t *)malloc(sizeof(node_unary_t));
		if(unary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
			return NULL;
		}
		memset(unary2, 0, sizeof(node_unary_t));

		unary2->right = node_clone(node1, unary1->right);
		if (unary2->right == NULL)
		{
			return NULL;
		}

		node1->value = unary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_RETURN)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_unary_t *unary1 = (node_unary_t *)node->value;

		node_unary_t *unary2 = (node_unary_t *)malloc(sizeof(node_unary_t));
		if(unary2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
			return NULL;
		}
		memset(unary2, 0, sizeof(node_unary_t));

		unary2->right = node_clone(node1, unary1->right);
		if (unary2->right == NULL)
		{
			return NULL;
		}

		node1->value = unary2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_IF)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_if_t *if1 = (node_if_t *)node->value;

		node_if_t *if2 = (node_if_t *)malloc(sizeof(node_if_t));
		if(if2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_if_t));
			return NULL;
		}
		memset(if2, 0, sizeof(node_if_t));

		if (if1->condition != NULL)
		{
			if2->condition = node_clone(node1, if1->condition);
			if (if2->condition == NULL)
			{
				return NULL;
			}
		}

		if (if1->then_body != NULL)
		{
			if2->then_body = node_clone(node1, if1->then_body);
			if (if2->then_body == NULL)
			{
				return NULL;
			}
		}

		if (if1->else_body != NULL)
		{
			if2->else_body = node_clone(node1, if1->else_body);
			if (if2->else_body == NULL)
			{
				return NULL;
			}
		}

		node1->value = if2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_INITIALIZER)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_block_t *block1 = (node_block_t *)node->value;

		node_block_t *block2 = (node_block_t *)malloc(sizeof(node_block_t));
		if(block2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
			return NULL;
		}
		memset(block2, 0, sizeof(node_block_t));
		
		list_t *list1 = list_create();
		if (list1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(list_t));
			return NULL;
		}

		ilist_t *a1;
		for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
		{
			node_t *item1 = (node_t *)a1->value;
			node_t *item2 = node_clone(node1, item1);
			if (item2 == NULL)
			{
				return NULL;
			}

			ilist_t *r1 = list_rpush(list1, item2);
			if (r1 == NULL)
			{
				return NULL;
			}
		}

		block2->list = list1;

		node1->value = block2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_INCREMENTOR)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_block_t *block1 = (node_block_t *)node->value;

		node_block_t *block2 = (node_block_t *)malloc(sizeof(node_block_t));
		if(block2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
			return NULL;
		}
		memset(block2, 0, sizeof(node_block_t));
		
		list_t *list1 = list_create();
		if (list1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(list_t));
			return NULL;
		}

		ilist_t *a1;
		for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
		{
			node_t *item1 = (node_t *)a1->value;
			node_t *item2 = node_clone(node1, item1);
			if (item2 == NULL)
			{
				return NULL;
			}

			ilist_t *r1 = list_rpush(list1, item2);
			if (r1 == NULL)
			{
				return NULL;
			}
		}

		block2->list = list1;

		node1->value = block2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_FOR)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_for_t *for1 = (node_for_t *)node->value;

		node_for_t *for2 = (node_for_t *)malloc(sizeof(node_for_t));
		if(for2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_for_t));
			return NULL;
		}
		memset(for2, 0, sizeof(node_for_t));

		if (for1->key != NULL)
		{
			for2->key = node_clone(node1, for1->key);
			if (for2->key == NULL)
			{
				return NULL;
			}
		}

		if (for1->initializer != NULL)
		{
			for2->initializer = node_clone(node1, for1->initializer);
			if (for2->initializer == NULL)
			{
				return NULL;
			}
		}

		if (for1->condition != NULL)
		{
			for2->condition = node_clone(node1, for1->condition);
			if (for2->condition == NULL)
			{
				return NULL;
			}
		}

		if (for1->incrementor != NULL)
		{
			for2->incrementor = node_clone(node1, for1->incrementor);
			if (for2->incrementor == NULL)
			{
				return NULL;
			}
		}

		if (for1->body != NULL)
		{
			for2->body = node_clone(node1, for1->body);
			if (for2->body == NULL)
			{
				return NULL;
			}
		}

		node1->value = for2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_FORIN)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_forin_t *for1 = (node_forin_t *)node->value;

		node_forin_t *for2 = (node_forin_t *)malloc(sizeof(node_forin_t));
		if(for2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_forin_t));
			return NULL;
		}
		memset(for2, 0, sizeof(node_forin_t));

		if (for1->key != NULL)
		{
			for2->key = node_clone(node1, for1->key);
			if (for2->key == NULL)
			{
				return NULL;
			}
		}

		if (for1->initializer != NULL)
		{
			for2->initializer = node_clone(node1, for1->initializer);
			if (for2->initializer == NULL)
			{
				return NULL;
			}
		}

		if (for1->expression != NULL)
		{
			for2->expression = node_clone(node1, for1->expression);
			if (for2->expression == NULL)
			{
				return NULL;
			}
		}

		if (for1->body != NULL)
		{
			for2->body = node_clone(node1, for1->body);
			if (for2->body == NULL)
			{
				return NULL;
			}
		}

		node1->value = for2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_CATCH)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_catch_t *catch1 = (node_catch_t *)node->value;

		node_catch_t *catch2 = (node_catch_t *)malloc(sizeof(node_catch_t));
		if(catch2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_catch_t));
			return NULL;
		}
		memset(catch2, 0, sizeof(node_catch_t));

		if (catch1->parameters != NULL)
		{
			catch2->parameters = node_clone(node1, catch1->parameters);
			if (catch2->parameters == NULL)
			{
				return NULL;
			}
		}

		if (catch1->body != NULL)
		{
			catch2->body = node_clone(node1, catch1->body);
			if (catch2->body == NULL)
			{
				return NULL;
			}
		}

		node1->value = catch2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_CATCHS)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_block_t *block1 = (node_block_t *)node->value;

		node_block_t *block2 = (node_block_t *)malloc(sizeof(node_block_t));
		if(block2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
			return NULL;
		}
		memset(block2, 0, sizeof(node_block_t));
		
		list_t *list1 = list_create();
		if (list1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(list_t));
			return NULL;
		}

		ilist_t *a1;
		for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
		{
			node_t *item1 = (node_t *)a1->value;
			node_t *item2 = node_clone(node1, item1);
			if (item2 == NULL)
			{
				return NULL;
			}

			ilist_t *r1 = list_rpush(list1, item2);
			if (r1 == NULL)
			{
				return NULL;
			}
		}

		block2->list = list1;

		node1->value = block2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_TRY)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_try_t *try1 = (node_try_t *)node->value;

		node_try_t *try2 = (node_try_t *)malloc(sizeof(node_try_t));
		if(try2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_try_t));
			return NULL;
		}
		memset(try2, 0, sizeof(node_try_t));

		if (try1->catchs != NULL)
		{
			try2->catchs = node_clone(node1, try1->catchs);
			if (try2->catchs == NULL)
			{
				return NULL;
			}
		}

		if (try1->body != NULL)
		{
			try2->body = node_clone(node1, try1->body);
			if (try2->body == NULL)
			{
				return NULL;
			}
		}

		node1->value = try2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_SET)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_block_t *block1 = (node_block_t *)node->value;

		node_block_t *block2 = (node_block_t *)malloc(sizeof(node_block_t));
		if(block2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
			return NULL;
		}
		memset(block2, 0, sizeof(node_block_t));
		
		list_t *list1 = list_create();
		if (list1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(list_t));
			return NULL;
		}

		ilist_t *a1;
		for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
		{
			node_t *item1 = (node_t *)a1->value;
			node_t *item2 = node_clone(node1, item1);
			if (item2 == NULL)
			{
				return NULL;
			}

			ilist_t *r1 = list_rpush(list1, item2);
			if (r1 == NULL)
			{
				return NULL;
			}
		}

		block2->list = list1;

		node1->value = block2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_VAR)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_var_t *var1 = (node_var_t *)node->value;

		node_var_t *var2 = (node_var_t *)malloc(sizeof(node_var_t));
		if(var2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_var_t));
			return NULL;
		}
		memset(var2, 0, sizeof(node_var_t));

		var2->flag = var1->flag;

		if (var1->key != NULL)
		{
			var2->key = node_clone(node1, var1->key);
			if (var2->key == NULL)
			{
				return NULL;
			}
		}

		if (var1->type != NULL)
		{
			var2->type = node_clone(node1, var1->type);
			if (var2->type == NULL)
			{
				return NULL;
			}
		}

		if (var1->value != NULL)
		{
			var2->value = node_clone(node1, var1->value);
			if (var2->value == NULL)
			{
				return NULL;
			}
		}

		node1->value = var2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_ARGUMENT)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_argument_t *argument1 = (node_argument_t *)node->value;

		node_argument_t *argument2 = (node_argument_t *)malloc(sizeof(node_argument_t));
		if(argument2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_argument_t));
			return NULL;
		}
		memset(argument2, 0, sizeof(node_argument_t));

		if (argument1->key != NULL)
		{
			argument2->key = node_clone(node1, argument1->key);
			if (argument2->key == NULL)
			{
				return NULL;
			}
		}

		if (argument1->value != NULL)
		{
			argument2->value = node_clone(node1, argument1->value);
			if (argument2->value == NULL)
			{
				return NULL;
			}
		}

		node1->value = argument2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_ARGUMENTS)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_block_t *block1 = (node_block_t *)node->value;

		node_block_t *block2 = (node_block_t *)malloc(sizeof(node_block_t));
		if(block2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
			return NULL;
		}
		memset(block2, 0, sizeof(node_block_t));
		
		list_t *list1 = list_create();
		if (list1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(list_t));
			return NULL;
		}

		ilist_t *a1;
		for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
		{
			node_t *item1 = (node_t *)a1->value;
			node_t *item2 = node_clone(node1, item1);
			if (item2 == NULL)
			{
				return NULL;
			}

			ilist_t *r1 = list_rpush(list1, item2);
			if (r1 == NULL)
			{
				return NULL;
			}
		}

		block2->list = list1;

		node1->value = block2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_PARAMETER)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_parameter_t *parameter1 = (node_parameter_t *)node->value;

		node_parameter_t *parameter2 = (node_parameter_t *)malloc(sizeof(node_parameter_t));
		if(parameter2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_parameter_t));
			return NULL;
		}
		memset(parameter2, 0, sizeof(node_parameter_t));

		parameter2->flag = parameter1->flag;

		if (parameter1->key != NULL)
		{
			parameter2->key = node_clone(node1, parameter1->key);
			if (parameter2->key == NULL)
			{
				return NULL;
			}
		}

		if (parameter1->type != NULL)
		{
			parameter2->type = node_clone(node1, parameter1->type);
			if (parameter2->type == NULL)
			{
				return NULL;
			}
		}

		if (parameter1->value != NULL)
		{
			parameter2->value = node_clone(node1, parameter1->value);
			if (parameter2->value == NULL)
			{
				return NULL;
			}
		}

		node1->value = parameter2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_PARAMETERS)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_block_t *block1 = (node_block_t *)node->value;

		node_block_t *block2 = (node_block_t *)malloc(sizeof(node_block_t));
		if(block2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
			return NULL;
		}
		memset(block2, 0, sizeof(node_block_t));
		
		list_t *list1 = list_create();
		if (list1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(list_t));
			return NULL;
		}

		ilist_t *a1;
		for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
		{
			node_t *item1 = (node_t *)a1->value;
			node_t *item2 = node_clone(node1, item1);
			if (item2 == NULL)
			{
				return NULL;
			}

			ilist_t *r1 = list_rpush(list1, item2);
			if (r1 == NULL)
			{
				return NULL;
			}
		}

		block2->list = list1;

		node1->value = block2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_FIELD)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_field_t *field1 = (node_field_t *)node->value;

		node_field_t *field2 = (node_field_t *)malloc(sizeof(node_field_t));
		if(field2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_field_t));
			return NULL;
		}
		memset(field2, 0, sizeof(node_field_t));

		if (field1->key != NULL)
		{
			field2->key = node_clone(node1, field1->key);
			if (field2->key == NULL)
			{
				return NULL;
			}
		}

		if (field1->value != NULL)
		{
			field2->value = node_clone(node1, field1->value);
			if (field2->value == NULL)
			{
				return NULL;
			}
		}

		node1->value = field2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_FIELDS)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_block_t *block1 = (node_block_t *)node->value;

		node_block_t *block2 = (node_block_t *)malloc(sizeof(node_block_t));
		if(block2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
			return NULL;
		}
		memset(block2, 0, sizeof(node_block_t));
		
		list_t *list1 = list_create();
		if (list1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(list_t));
			return NULL;
		}

		ilist_t *a1;
		for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
		{
			node_t *item1 = (node_t *)a1->value;
			node_t *item2 = node_clone(node1, item1);
			if (item2 == NULL)
			{
				return NULL;
			}

			ilist_t *r1 = list_rpush(list1, item2);
			if (r1 == NULL)
			{
				return NULL;
			}
		}

		block2->list = list1;

		node1->value = block2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_HERITAGES)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_block_t *block1 = (node_block_t *)node->value;

		node_block_t *block2 = (node_block_t *)malloc(sizeof(node_block_t));
		if(block2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
			return NULL;
		}
		memset(block2, 0, sizeof(node_block_t));
		
		list_t *list1 = list_create();
		if (list1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(list_t));
			return NULL;
		}

		ilist_t *a1;
		for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
		{
			node_t *item1 = (node_t *)a1->value;
			node_t *item2 = node_clone(node1, item1);
			if (item2 == NULL)
			{
				return NULL;
			}

			ilist_t *r1 = list_rpush(list1, item2);
			if (r1 == NULL)
			{
				return NULL;
			}
		}

		block2->list = list1;

		node1->value = block2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_GENERICS)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_block_t *block1 = (node_block_t *)node->value;

		node_block_t *block2 = (node_block_t *)malloc(sizeof(node_block_t));
		if(block2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
			return NULL;
		}
		memset(block2, 0, sizeof(node_block_t));
		
		list_t *list1 = list_create();
		if (list1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(list_t));
			return NULL;
		}

		ilist_t *a1;
		for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
		{
			node_t *item1 = (node_t *)a1->value;
			node_t *item2 = node_clone(node1, item1);
			if (item2 == NULL)
			{
				return NULL;
			}

			ilist_t *r1 = list_rpush(list1, item2);
			if (r1 == NULL)
			{
				return NULL;
			}
		}

		block2->list = list1;

		node1->value = block2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_HERITAGE)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_heritage_t *heritage1 = (node_heritage_t *)node->value;

		node_heritage_t *heritage2 = (node_heritage_t *)malloc(sizeof(node_heritage_t));
		if(heritage2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_heritage_t));
			return NULL;
		}
		memset(heritage2, 0, sizeof(node_heritage_t));
		
		if (heritage1->key != NULL)
		{
			heritage2->key = node_clone(node1, heritage1->key);
			if (heritage2->key == NULL)
			{
				return NULL;
			}
		}

		if (heritage1->type != NULL)
		{
			heritage2->type = node_clone(node1, heritage1->type);
			if (heritage2->type == NULL)
			{
				return NULL;
			}
		}

		if (heritage1->value_update != NULL)
		{
			heritage2->value_update = heritage1->value_update;
		}

		node1->value = heritage2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_GENERIC)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_generic_t *generic1 = (node_generic_t *)node->value;

		node_generic_t *generic2 = (node_generic_t *)malloc(sizeof(node_generic_t));
		if(generic2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_generic_t));
			return NULL;
		}
		memset(generic2, 0, sizeof(node_generic_t));
		
		if (generic1->key != NULL)
		{
			generic2->key = node_clone(node1, generic1->key);
			if (generic2->key == NULL)
			{
				return NULL;
			}
		}

		if (generic1->type != NULL)
		{
			generic2->type = node_clone(node1, generic1->type);
			if (generic2->type == NULL)
			{
				return NULL;
			}
		}

		if (generic1->value != NULL)
		{
			generic2->value = node_clone(node1, generic1->value);
			if (generic2->value == NULL)
			{
				return NULL;
			}
		}

		if (generic1->value_update != NULL)
		{
			generic2->value_update = generic1->value_update;
		}

		node1->value = generic2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_BLOCK)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_block_t *block1 = (node_block_t *)node->value;

		node_block_t *block2 = (node_block_t *)malloc(sizeof(node_block_t));
		if(block2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
			return NULL;
		}
		memset(block2, 0, sizeof(node_block_t));
		
		list_t *list1 = list_create();
		if (list1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(list_t));
			return NULL;
		}

		ilist_t *a1;
		for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
		{
			node_t *item1 = (node_t *)a1->value;
			node_t *item2 = node_clone(node1, item1);
			if (item2 == NULL)
			{
				return NULL;
			}

			ilist_t *r1 = list_rpush(list1, item2);
			if (r1 == NULL)
			{
				return NULL;
			}
		}

		block2->list = list1;

		node1->value = block2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_BODY)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_block_t *block1 = (node_block_t *)node->value;

		node_block_t *block2 = (node_block_t *)malloc(sizeof(node_block_t));
		if(block2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
			return NULL;
		}
		memset(block2, 0, sizeof(node_block_t));
		
		list_t *list1 = list_create();
		if (list1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(list_t));
			return NULL;
		}

		ilist_t *a1;
		for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
		{
			node_t *item1 = (node_t *)a1->value;
			node_t *item2 = node_clone(node1, item1);
			if (item2 == NULL)
			{
				return NULL;
			}

			ilist_t *r1 = list_rpush(list1, item2);
			if (r1 == NULL)
			{
				return NULL;
			}
		}

		block2->list = list1;

		node1->value = block2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_FUN)
	{
		node_fun_t *fun1 = (node_fun_t *)node->value;

		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		if ((fun1->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
		{
			node1->parent = node->parent;
		}
		else
		{
			node1->parent = parent;
		}

		node_fun_t *fun2 = (node_fun_t *)malloc(sizeof(node_fun_t));
		if(fun2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_fun_t));
			return NULL;
		}
		memset(fun2, 0, sizeof(node_fun_t));

		fun2->flag = fun1->flag;

		if (fun1->key != NULL)
		{
			fun2->key = node_clone(node1, fun1->key);
			if (fun2->key == NULL)
			{
				return NULL;
			}
		}

		if (fun1->generics != NULL)
		{
			fun2->generics = node_clone(node1, fun1->generics);
			if (fun2->generics == NULL)
			{
				return NULL;
			}
		}

		if (fun1->parameters != NULL)
		{
			fun2->parameters = node_clone(node1, fun1->parameters);
			if (fun2->parameters == NULL)
			{
				return NULL;
			}
		}

		if (fun1->result != NULL)
		{
			fun2->result = node_clone(node1, fun1->result);
			if (fun2->result == NULL)
			{
				return NULL;
			}
		}

		if (fun1->body != NULL)
		{
			fun2->body = node_clone(node1, fun1->body);
			if (fun2->body == NULL)
			{
				return NULL;
			}
		}

		if (fun1->annotation != NULL)
		{
			fun2->annotation = node_clone(node1, fun1->annotation);
			if (fun2->annotation == NULL)
			{
				return NULL;
			}
		}

		node1->value = fun2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_LAMBDA)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_lambda_t *fun1 = (node_lambda_t *)node->value;

		node_lambda_t *fun2 = (node_lambda_t *)malloc(sizeof(node_lambda_t));
		if(fun2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_lambda_t));
			return NULL;
		}
		memset(fun2, 0, sizeof(node_lambda_t));

		if (fun1->key != NULL)
		{
			fun2->key = node_clone(node1, fun1->key);
			if (fun2->key == NULL)
			{
				return NULL;
			}
		}

		if (fun1->generics != NULL)
		{
			fun2->generics = node_clone(node1, fun1->generics);
			if (fun2->generics == NULL)
			{
				return NULL;
			}
		}

		if (fun1->parameters != NULL)
		{
			fun2->parameters = node_clone(node1, fun1->parameters);
			if (fun2->parameters == NULL)
			{
				return NULL;
			}
		}

		if (fun1->body != NULL)
		{
			fun2->body = node_clone(node1, fun1->body);
			if (fun2->body == NULL)
			{
				return NULL;
			}
		}

		node1->value = fun2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_FN)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_fn_t *fun1 = (node_fn_t *)node->value;

		node_fn_t *fun2 = (node_fn_t *)malloc(sizeof(node_fn_t));
		if(fun2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_fn_t));
			return NULL;
		}
		memset(fun2, 0, sizeof(node_fn_t));

		if (fun1->generics != NULL)
		{
			fun2->generics = node_clone(node1, fun1->generics);
			if (fun2->generics == NULL)
			{
				return NULL;
			}
		}

		if (fun1->parameters != NULL)
		{
			fun2->parameters = node_clone(node1, fun1->parameters);
			if (fun2->parameters == NULL)
			{
				return NULL;
			}
		}

		if (fun1->result != NULL)
		{
			fun2->result = node_clone(node1, fun1->result);
			if (fun2->result == NULL)
			{
				return NULL;
			}
		}

		node1->value = fun2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_PROPERTY)
	{
		node_property_t *property1 = (node_property_t *)node->value;

		if ((property1->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
		{
			return node;
		}

		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_property_t *property2 = (node_property_t *)malloc(sizeof(node_property_t));
		if(property2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_property_t));
			return NULL;
		}
		memset(property2, 0, sizeof(node_property_t));

		property2->flag = property1->flag;

		if (property1->key != NULL)
		{
			property2->key = node_clone(node1, property1->key);
			if (property2->key == NULL)
			{
				return NULL;
			}
		}

		if (property1->type != NULL)
		{
			property2->type = node_clone(node1, property1->type);
			if (property2->type == NULL)
			{
				return NULL;
			}
		}

		if (property1->value != NULL)
		{
			property2->value = node_clone(node1, property1->value);
			if (property2->value == NULL)
			{
				return NULL;
			}
		}

		if (property1->annotation != NULL)
		{
			property2->annotation = node_clone(node1, property1->annotation);
			if (property2->annotation == NULL)
			{
				return NULL;
			}
		}

		node1->value = property2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_ANNOTATION)
	{
		node_note_t *note1 = (node_note_t *)node->value;

		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_note_t *note2 = (node_note_t *)malloc(sizeof(node_note_t));
		if(note2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_note_t));
			return NULL;
		}
		memset(note2, 0, sizeof(node_note_t));

		if (note1->key != NULL)
		{
			note2->key = node_clone(node1, note1->key);
			if (note2->key == NULL)
			{
				return NULL;
			}
		}

		if (note1->arguments != NULL)
		{
			note2->arguments = node_clone(node1, note1->arguments);
			if (note2->arguments == NULL)
			{
				return NULL;
			}
		}

		if (note1->next != NULL)
		{
			note2->next = node_clone(node1, note1->next);
			if (note2->next == NULL)
			{
				return NULL;
			}
		}

		node1->value = note2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_ENTITY)
	{
		node_entity_t *enteiry1 = (node_entity_t *)node->value;

		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_entity_t *enteiry2 = (node_entity_t *)malloc(sizeof(node_entity_t));
		if(enteiry2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_entity_t));
			return NULL;
		}
		memset(enteiry2, 0, sizeof(node_entity_t));

		enteiry2->flag = enteiry1->flag;

		if (enteiry1->key != NULL)
		{
			enteiry2->key = node_clone(node1, enteiry1->key);
			if (enteiry2->key == NULL)
			{
				return NULL;
			}
		}

		if (enteiry1->type != NULL)
		{
			enteiry2->type = node_clone(node1, enteiry1->type);
			if (enteiry2->type == NULL)
			{
				return NULL;
			}
		}

		if (enteiry1->value != NULL)
		{
			enteiry2->value = node_clone(node1, enteiry1->value);
			if (enteiry2->value == NULL)
			{
				return NULL;
			}
		}

		node1->value = enteiry2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_CLASS)
	{
		node_class_t *class1 = (node_class_t *)node->value;

		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		if ((class1->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
		{
			node1->parent = node->parent;
		}
		else
		{
			node1->parent = parent;
		}

		node_class_t *class2 = (node_class_t *)malloc(sizeof(node_class_t));
		if(class2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_class_t));
			return NULL;
		}
		memset(class2, 0, sizeof(node_class_t));

		class2->flag = class1->flag;

		if (class1->key != NULL)
		{
			class2->key = node_clone(node1, class1->key);
			if (class2->key == NULL)
			{
				return NULL;
			}
		}

		if (class1->generics != NULL)
		{
			class2->generics = node_clone(node1, class1->generics);
			if (class2->generics == NULL)
			{
				return NULL;
			}
		}

		if (class1->heritages != NULL)
		{
			class2->heritages = node_clone(node1, class1->heritages);
			if (class2->heritages == NULL)
			{
				return NULL;
			}
		}

		if (class1->block != NULL)
		{
			class2->block = node_clone(node1, class1->block);
			if (class2->block == NULL)
			{
				return NULL;
			}
		}

		if (class1->annotation != NULL)
		{
			class2->annotation = node_clone(node1, class1->annotation);
			if (class2->annotation == NULL)
			{
				return NULL;
			}
		}

		node1->value = class2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_PAIR)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_pair_t *argument1 = (node_pair_t *)node->value;

		node_pair_t *argument2 = (node_pair_t *)malloc(sizeof(node_pair_t));
		if(argument2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_pair_t));
			return NULL;
		}
		memset(argument2, 0, sizeof(node_pair_t));

		if (argument1->key != NULL)
		{
			argument2->key = node_clone(node1, argument1->key);
			if (argument2->key == NULL)
			{
				return NULL;
			}
		}

		if (argument1->value != NULL)
		{
			argument2->value = node_clone(node1, argument1->value);
			if (argument2->value == NULL)
			{
				return NULL;
			}
		}

		node1->value = argument2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_MEMBER)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_member_t *member1 = (node_member_t *)node->value;

		node_member_t *member2 = (node_member_t *)malloc(sizeof(node_member_t));
		if(member2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_member_t));
			return NULL;
		}
		memset(member2, 0, sizeof(node_member_t));

		if (member1->key != NULL)
		{
			member2->key = node_clone(node1, member1->key);
			if (member2->key == NULL)
			{
				return NULL;
			}
		}

		if (member1->value != NULL)
		{
			member2->value = node_clone(node1, member1->value);
			if (member2->value == NULL)
			{
				return NULL;
			}
		}

		node1->value = member2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_MEMBERS)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_block_t *block1 = (node_block_t *)node->value;

		node_block_t *block2 = (node_block_t *)malloc(sizeof(node_block_t));
		if(block2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
			return NULL;
		}
		memset(block2, 0, sizeof(node_block_t));
		
		list_t *list1 = list_create();
		if (list1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(list_t));
			return NULL;
		}

		ilist_t *a1;
		for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
		{
			node_t *item1 = (node_t *)a1->value;
			node_t *item2 = node_clone(node1, item1);
			if (item2 == NULL)
			{
				return NULL;
			}

			ilist_t *r1 = list_rpush(list1, item2);
			if (r1 == NULL)
			{
				return NULL;
			}
		}

		block2->list = list1;

		node1->value = block2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_ENUM)
	{
		node_enum_t *enum1 = (node_enum_t *)node->value;

		if ((enum1->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
		{
			return node;
		}

		if ((enum1->flag & SYNTAX_MODIFIER_STATIC) == SYNTAX_MODIFIER_STATIC)
		{
			return node;
		}

		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_enum_t *enum2 = (node_enum_t *)malloc(sizeof(node_enum_t));
		if(enum2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_enum_t));
			return NULL;
		}
		memset(enum2, 0, sizeof(node_enum_t));

		enum2->flag = enum1->flag;

		if (enum1->key != NULL)
		{
			enum2->key = node_clone(node1, enum1->key);
			if (enum2->key == NULL)
			{
				return NULL;
			}
		}

		if (enum1->block != NULL)
		{
			enum2->block = node_clone(node1, enum1->block);
			if (enum2->block == NULL)
			{
				return NULL;
			}
		}

		if (enum1->annotation != NULL)
		{
			enum2->annotation = node_clone(node1, enum1->annotation);
			if (enum2->annotation == NULL)
			{
				return NULL;
			}
		}

		node1->value = enum2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_PACKAGE)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_package_t *package1 = (node_package_t *)node->value;

		node_package_t *package2 = (node_package_t *)malloc(sizeof(node_package_t));
		if(package2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_package_t));
			return NULL;
		}
		memset(package2, 0, sizeof(node_package_t));

		if (package1->key != NULL)
		{
			package2->key = node_clone(node1, package1->key);
			if (package2->key == NULL)
			{
				return NULL;
			}
		}

		if (package1->generics != NULL)
		{
			package2->generics = node_clone(node1, package1->generics);
			if (package2->generics == NULL)
			{
				return NULL;
			}
		}

		if (package1->address != NULL)
		{
			package2->address = node_clone(node1, package1->address);
			if (package2->address == NULL)
			{
				return NULL;
			}
		}

		node1->value = package2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_PACKAGES)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_block_t *block1 = (node_block_t *)node->value;

		node_block_t *block2 = (node_block_t *)malloc(sizeof(node_block_t));
		if(block2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
			return NULL;
		}
		memset(block2, 0, sizeof(node_block_t));
		
		list_t *list1 = list_create();
		if (list1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(list_t));
			return NULL;
		}

		ilist_t *a1;
		for (a1 = block1->list->begin;a1 != block1->list->end;a1 = a1->next)
		{
			node_t *item1 = (node_t *)a1->value;
			node_t *item2 = node_clone(node1, item1);
			if (item2 == NULL)
			{
				return NULL;
			}

			ilist_t *r1 = list_rpush(list1, item2);
			if (r1 == NULL)
			{
				return NULL;
			}
		}

		block2->list = list1;

		node1->value = block2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_USING)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_using_t *using1 = (node_using_t *)node->value;

		node_using_t *using2 = (node_using_t *)malloc(sizeof(node_using_t));
		if(using2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_using_t));
			return NULL;
		}
		memset(using2, 0, sizeof(node_using_t));

		if (using1->path != NULL)
		{
			using2->path = node_clone(node1, using1->path);
			if (using2->path == NULL)
			{
				return NULL;
			}
		}

		if (using1->packages != NULL)
		{
			using2->packages = node_clone(node1, using1->packages);
			if (using2->packages == NULL)
			{
				return NULL;
			}
		}

		node1->value = using2;

		return node1;
	}
	else
	if (node->kind == NODE_KIND_MODULE)
	{
		node_t *node1 = (node_t *)malloc(sizeof(node_t));
		if(node1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_t));
			return NULL;
		}
		memset(node1, 0, sizeof(node_t));

		node1->id = node->id;
		node1->position = node->position;
		node1->parent = parent;
		node1->kind = node->kind;
		node1->flag = NODE_FLAG_TEMPORARY;

		node_module_t *module1 = (node_module_t *)node->value;

		node_module_t *module2 = (node_module_t *)malloc(sizeof(node_module_t));
		if(module2 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_module_t));
			return NULL;
		}
		memset(module2, 0, sizeof(node_module_t));
		
		list_t *list1 = list_create();
		if (list1 == NULL)
		{
			fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(list_t));
			return NULL;
		}

		ilist_t *a1;
		for (a1 = module1->items->begin;a1 != module1->items->end;a1 = a1->next)
		{
			node_t *item1 = (node_t *)a1->value;
			node_t *item2 = node_clone(node1, item1);
			if (item2 == NULL)
			{
				return NULL;
			}

			ilist_t *r1 = list_rpush(list1, item2);
			if (r1 == NULL)
			{
				return NULL;
			}
		}

		module2->items = list1;
		module2->path = module1->path;

		node1->value = module2;

		return node1;
	}
	
	return NULL;
}

void
node_prug(node_t *node)
{
}

node_t *
node_make_id(node_t *node, char *value)
{
	node_basic_t *basic = (node_basic_t *)malloc(sizeof(node_basic_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_basic_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_basic_t));
	basic->value = value;

	node_update(node, NODE_KIND_ID, basic);
	return node;
}


node_t *
node_make_number(node_t *node, char *value)
{
	node_basic_t *basic = (node_basic_t *)malloc(sizeof(node_basic_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_basic_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_basic_t));
	basic->value = value;

	node_update(node, NODE_KIND_NUMBER, basic);
	return node;
}

node_t *
node_make_char(node_t *node, char *value)
{
	node_basic_t *basic = (node_basic_t *)malloc(sizeof(node_basic_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_basic_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_basic_t));
	basic->value = value;
	
	node_update(node, NODE_KIND_CHAR, basic);
	return node;
}

node_t *
node_make_string(node_t *node, char *value)
{
	node_basic_t *basic = (node_basic_t *)malloc(sizeof(node_basic_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_basic_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_basic_t));
	basic->value = value;
	
	node_update(node, NODE_KIND_STRING, basic);
	return node;
}

node_t *
node_make_null(node_t *node)
{
	node_basic_t *basic = (node_basic_t *)malloc(sizeof(node_basic_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_basic_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_basic_t));
	basic->value = NULL;
	
	node_update(node, NODE_KIND_NULL, basic);
	return node;
}

node_t *
node_make_this(node_t *node)
{
	node_basic_t *basic = (node_basic_t *)malloc(sizeof(node_basic_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_basic_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_basic_t));
	basic->value = NULL;
	
	node_update(node, NODE_KIND_THIS, basic);
	return node;
}


node_t *
node_make_array(node_t *node, list_t *list)
{
	node_block_t *basic = (node_block_t *)malloc(sizeof(node_block_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_block_t));
	basic->list = list;
	
	node_update(node, NODE_KIND_ARRAY, basic);
	return node;
}

node_t *
node_make_dictionary(node_t *node, list_t *list)
{
	node_block_t *basic = (node_block_t *)malloc(sizeof(node_block_t));
	if(basic == NULL){
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_block_t));
	basic->list = list;
	
	node_update(node, NODE_KIND_DICTIONARY, basic);
	return node;
}

node_t *
node_make_pseudonym(node_t *node, node_t *base, node_t *concepts)
{
	node_carrier_t *basic = (node_carrier_t *)malloc(sizeof(node_carrier_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_carrier_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_carrier_t));
	basic->base = base;
	basic->data = concepts;
	
	node_update(node, NODE_KIND_PSEUDONYM, basic);
	return node;
}


node_t *
node_make_typeof(node_t *node, node_t *right)
{
	node_unary_t *basic = (node_unary_t *)malloc(sizeof(node_unary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_unary_t));
	basic->right = right;
	
	node_update(node, NODE_KIND_TYPEOF, basic);
	return node;
}

node_t *
node_make_sizeof(node_t *node, node_t *right)
{
	node_unary_t *basic = (node_unary_t *)malloc(sizeof(node_unary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_unary_t));
	basic->right = right;
	
	node_update(node, NODE_KIND_SIZEOF, basic);
	return node;
}

node_t *
node_make_parenthesis(node_t *node, node_t *value)
{
	node_unary_t *basic = (node_unary_t *)malloc(sizeof(node_unary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_unary_t));
	basic->right = value;
	
	node_update(node, NODE_KIND_PARENTHESIS, basic);
	return node;
}

node_t *
node_make_call(node_t *node, node_t *base, node_t *arguments)
{
	node_carrier_t *basic = (node_carrier_t *)malloc(sizeof(node_carrier_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_carrier_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_carrier_t));
	basic->base = base;
	basic->data = arguments;
	
	node_update(node, NODE_KIND_CALL, basic);
	return node;
}

node_t *
node_make_item(node_t *node, node_t *base, node_t *arguments)
{
	node_carrier_t *basic = (node_carrier_t *)malloc(sizeof(node_carrier_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_carrier_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_carrier_t));
	basic->base = base;
	basic->data = arguments;
	
	node_update(node, NODE_KIND_ITEM, basic);
	return node;
}

node_t *
node_make_attribute(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_ATTRIBUTE, basic);
	return node;
}

node_t *
node_make_tilde(node_t *node, node_t *right)
{
	node_unary_t *basic = (node_unary_t *)malloc(sizeof(node_unary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_unary_t));
	basic->right = right;
	
	node_update(node, NODE_KIND_TILDE, basic);
	return node;
}

node_t *
node_make_not(node_t *node, node_t *right)
{
	node_unary_t *basic = (node_unary_t *)malloc(sizeof(node_unary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_unary_t));
	basic->right = right;
	
	node_update(node, NODE_KIND_NOT, basic);
	return node;
}

node_t *
node_make_neg(node_t *node, node_t *right)
{
	node_unary_t *basic = (node_unary_t *)malloc(sizeof(node_unary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_unary_t));
	basic->right = right;
	
	node_update(node, NODE_KIND_NEG, basic);
	return node;
}

node_t *
node_make_pos(node_t *node, node_t *right)
{
	node_unary_t *basic = (node_unary_t *)malloc(sizeof(node_unary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_unary_t));
	basic->right = right;
	
	node_update(node, NODE_KIND_POS, basic);
	return node;
}

node_t *
node_make_pow(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_POW, basic);
	return node;
}

node_t *
node_make_epi(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_EPI, basic);
	return node;
}

node_t *
node_make_mul(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_MUL, basic);
	return node;
}

node_t *
node_make_div(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_DIV, basic);
	return node;
}

node_t *
node_make_mod(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_MOD, basic);
	return node;
}

node_t *
node_make_plus(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_PLUS, basic);
	return node;
}

node_t *
node_make_minus(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_MINUS, basic);
	return node;
}

node_t *
node_make_shl(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_SHL, basic);
	return node;
}

node_t *
node_make_shr(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_SHR, basic);
	return node;
}

node_t *
node_make_lt(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_LT, basic);
	return node;
}

node_t *
node_make_le(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_LE, basic);
	return node;
}

node_t *
node_make_gt(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_GT, basic);
	return node;
}

node_t *
node_make_ge(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_GE, basic);
	return node;
}

node_t *
node_make_eq(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_EQ, basic);
	return node;
}

node_t *
node_make_neq(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_NEQ, basic);
	return node;
}

node_t *
node_make_and(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_AND, basic);
	return node;
}

node_t *
node_make_xor(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_XOR, basic);
	return node;
}

node_t *
node_make_or(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_OR, basic);
	return node;
}

node_t *
node_make_land(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_LAND, basic);
	return node;
}

node_t *
node_make_lor(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_LOR, basic);
	return node;
}

node_t *
node_make_assign(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_ASSIGN, basic);
	return node;
}

node_t *
node_make_define(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_DEFINE, basic);
	return node;
}

node_t *
node_make_add_assign(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_ADD_ASSIGN, basic);
	return node;
}

node_t *
node_make_sub_assign(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_SUB_ASSIGN, basic);
	return node;
}

node_t *
node_make_div_assign(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_DIV_ASSIGN, basic);
	return node;
}

node_t *
node_make_mul_assign(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_MUL_ASSIGN, basic);
	return node;
}

node_t *
node_make_mod_assign(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_MOD_ASSIGN, basic);
	return node;
}

node_t *
node_make_and_assign(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_AND_ASSIGN, basic);
	return node;
}

node_t *
node_make_or_assign(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_OR_ASSIGN, basic);
	return node;
}

node_t *
node_make_shl_assign(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_SHL_ASSIGN, basic);
	return node;
}

node_t *
node_make_shr_assign(node_t *node, node_t *left, node_t *right)
{
	node_binary_t *basic = (node_binary_t *)malloc(sizeof(node_binary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_binary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_binary_t));
	basic->left = left;
	basic->right = right;
	
	node_update(node, NODE_KIND_SHR_ASSIGN, basic);
	return node;
}


node_t *
node_make_break(node_t *node, node_t *expression)
{
	node_unary_t *basic = (node_unary_t *)malloc(sizeof(node_unary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_unary_t));
	basic->right = expression;

	node_update(node, NODE_KIND_BREAK, basic);
	return node;
}

node_t *
node_make_continue(node_t *node, node_t *expression)
{
	node_unary_t *basic = (node_unary_t *)malloc(sizeof(node_unary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_unary_t));
	basic->right = expression;

	node_update(node, NODE_KIND_CONTINUE, basic);
	return node;
}

node_t *
node_make_return(node_t *node, node_t *expression)
{
	node_unary_t *basic = (node_unary_t *)malloc(sizeof(node_unary_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_unary_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_unary_t));
	basic->right = expression;
	
	node_update(node, NODE_KIND_RETURN, basic);
	return node;
}

node_t *
node_make_if(node_t *node, node_t *condition, node_t *then_body, node_t *else_body)
{
	node_if_t *basic;
	if(!(basic = (node_if_t *)malloc(sizeof(node_if_t))))
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_if_t));
		return NULL;
	}

	memset(basic, 0, sizeof(node_if_t));
	
	basic->condition = condition;
	basic->then_body = then_body;
	basic->else_body = else_body;
	
	node_update(node, NODE_KIND_IF, basic);
	return node;
}

node_t *
node_make_initializer(node_t *node, list_t *list)
{
	node_block_t *basic = (node_block_t *)malloc(sizeof(node_block_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_block_t));
	basic->list = list;
	
	node_update(node, NODE_KIND_INITIALIZER, basic);
	return node;
}

node_t *
node_make_incrementor(node_t *node, list_t *list)
{
	node_block_t *basic = (node_block_t *)malloc(sizeof(node_block_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_block_t));
	basic->list = list;
	
	node_update(node, NODE_KIND_INCREMENTOR, basic);
	return node;
}

node_t *
node_make_for(node_t *node, node_t *key, node_t *initializer, node_t *condition, node_t *incrementor, node_t *body)
{
	node_for_t *basic;
	if(!(basic = (node_for_t *)malloc(sizeof(node_for_t)))){
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_for_t));
		return NULL;
	}
	
	memset(basic, 0, sizeof(node_for_t));

	basic->key = key;
	basic->initializer = initializer;
	basic->condition = condition;
	basic->incrementor = incrementor;
	basic->body = body;
	
	node_update(node, NODE_KIND_FOR, basic);
	return node;
}

node_t *
node_make_forin(node_t *node, node_t *key, node_t *initializer, node_t *expression, node_t *body)
{
	node_forin_t *basic;
	if(!(basic = (node_forin_t *)malloc(sizeof(node_forin_t))))
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_forin_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_forin_t));
	
	basic->key = key;
	basic->initializer = initializer;
	basic->expression = expression;
	basic->body = body;
	
	node_update(node, NODE_KIND_FORIN, basic);
	return node;
}

node_t *
node_make_catch(node_t *node, node_t *parameters, node_t *body)
{
	node_catch_t *basic;
	if(!(basic = (node_catch_t *)malloc(sizeof(node_catch_t))))
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_catch_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_catch_t));
	basic->parameters = parameters;
	basic->body = body;
	
	node_update(node, NODE_KIND_CATCH, basic);
	return node;
}

node_t *
node_make_catchs(node_t *node, list_t *list)
{
	node_block_t *basic = (node_block_t *)malloc(sizeof(node_block_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_block_t));
	basic->list = list;
	
	node_update(node, NODE_KIND_CATCHS, basic);
	return node;
}

node_t *
node_make_try(node_t *node, node_t *body, node_t *catchs)
{
	node_try_t *basic;
	if(!(basic = (node_try_t *)malloc(sizeof(node_try_t))))
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_try_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_try_t));
	basic->body = body;
	basic->catchs = catchs;
	
	node_update(node, NODE_KIND_TRY, basic);
	return node;
}

node_t *
node_make_set(node_t *node, list_t *list)
{
	node_block_t *basic = (node_block_t *)malloc(sizeof(node_block_t));
	if(basic == NULL){
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_block_t));
	basic->list = list;
	
	node_update(node, NODE_KIND_SET, basic);
	return node;
}

node_t *
node_make_var(node_t *node, uint64_t flag, node_t *key, node_t *type, node_t *value){
	node_var_t *basic;
	if(!(basic = (node_var_t *)malloc(sizeof(node_var_t)))){
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_var_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_var_t));
	basic->flag = flag;
	basic->key = key;
	basic->type = type;
	basic->value = value;
	
	node_update(node, NODE_KIND_VAR, basic);
	return node;
}

node_t *
node_make_argument(node_t *node, node_t *key, node_t *value)
{
	node_argument_t *basic = (node_argument_t *)malloc(sizeof(node_argument_t));
	if(basic == NULL){
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_argument_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_argument_t));
	basic->key = key;
	basic->value = value;
	
	node_update(node, NODE_KIND_ARGUMENT, basic);
	return node;
}

node_t *
node_make_arguments(node_t *node, list_t *list)
{
	node_block_t *basic = (node_block_t *)malloc(sizeof(node_block_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_block_t));
	basic->list = list;
	
	node_update(node, NODE_KIND_ARGUMENTS, basic);
	return node;
}

node_t *
node_make_parameter(node_t *node, uint64_t flag, node_t *key, node_t *type, node_t *value){
	node_parameter_t *basic;
	if(!(basic = (node_parameter_t *)malloc(sizeof(node_parameter_t)))){
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_parameter_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_parameter_t));
	basic->flag = flag;
	basic->key = key;
	basic->type = type;
	basic->value = value;
	
	node_update(node, NODE_KIND_PARAMETER, basic);
	return node;
}

node_t *
node_make_parameters(node_t *node, list_t *list)
{
	node_block_t *basic = (node_block_t *)malloc(sizeof(node_block_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_block_t));
	basic->list = list;
	
	node_update(node, NODE_KIND_PARAMETERS, basic);
	return node;
}

node_t *
node_make_field(node_t *node, node_t *key, node_t *value)
{
	node_field_t *basic;
	if(!(basic = (node_field_t *)malloc(sizeof(node_field_t))))
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_field_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_field_t));
	basic->key = key;
	basic->value = value;
	
	node_update(node, NODE_KIND_FIELD, basic);
	return node;
}

node_t *
node_make_fields(node_t *node, list_t *list)
{
	node_block_t *basic = (node_block_t *)malloc(sizeof(node_block_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_block_t));
	basic->list = list;
	
	node_update(node, NODE_KIND_FIELDS, basic);
	return node;
}

node_t *
node_make_generic(node_t *node, node_t *key, node_t *type, node_t *value)
{
	node_generic_t *basic;
	if(!(basic = (node_generic_t *)malloc(sizeof(node_generic_t))))
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_generic_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_generic_t));
	basic->key = key;
	basic->type = type;
	basic->value = value;
	
	node_update(node, NODE_KIND_GENERIC, basic);
	return node;
}

node_t *
node_make_generics(node_t *node, list_t *list)
{
	node_block_t *basic = (node_block_t *)malloc(sizeof(node_block_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_block_t));
	basic->list = list;
	
	node_update(node, NODE_KIND_GENERICS, basic);
	return node;
}

node_t *
node_make_func(node_t *node, node_t *annotation, uint64_t flag, node_t *key, node_t *generics, node_t *parameters, node_t *result, node_t *body)
{
	node_fun_t *basic;
	if(!(basic = (node_fun_t *)malloc(sizeof(node_fun_t))))
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_fun_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_fun_t));
	basic->flag = flag;
	basic->annotation = annotation;
	basic->key = key;
	basic->generics = generics;
	basic->parameters = parameters;
	basic->result = result;
	basic->body = body;
	
	node_update(node, NODE_KIND_FUN, basic);
	return node;
}

node_t *
node_make_lambda(node_t *node, node_t *key, node_t *generics, node_t *parameters, node_t *body, node_t *result)
{
	node_lambda_t *basic;
	if(!(basic = (node_lambda_t *)malloc(sizeof(node_lambda_t))))
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_lambda_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_lambda_t));
	basic->key = key;
	basic->generics = generics;
	basic->parameters = parameters;
	basic->body = body;
	basic->result= result;
	
	node_update(node, NODE_KIND_LAMBDA, basic);
	return node;
}

node_t *
node_make_fn(node_t *node, node_t *generics, node_t *parameters, node_t *result)
{
	node_fn_t *basic = (node_fn_t *)malloc(sizeof(node_fn_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_fn_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_fn_t));
	basic->generics = generics;
	basic->parameters = parameters;
	basic->result = result;
	
	node_update(node, NODE_KIND_FN, basic);
	return node;
}

node_t *
node_make_property(node_t *node, node_t *annotation, uint64_t flag, node_t *key, node_t *type, node_t *value)
{
	node_property_t *basic = (node_property_t *)malloc(sizeof(node_property_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_property_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_property_t));
	basic->flag = flag;
	basic->annotation = annotation;
	basic->key = key;
	basic->type = type;
	basic->value = value;
	
	node_update(node, NODE_KIND_PROPERTY, basic);
	return node;
}

node_t *
node_make_entity(node_t *node, uint64_t flag, node_t *key, node_t *type, node_t *value)
{
	node_entity_t *basic = (node_entity_t *)malloc(sizeof(node_entity_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_entity_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_entity_t));
	basic->flag = flag;
	basic->key = key;
	basic->type = type;
	basic->value = value;
	
	node_update(node, NODE_KIND_ENTITY, basic);
	return node;
}

node_t *
node_make_pair(node_t *node, node_t *key, node_t *value)
{
	node_pair_t *basic = (node_pair_t *)malloc(sizeof(node_pair_t));
	if(basic != NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_pair_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_pair_t));
	basic->key = key;
	basic->value = value;
	
	node_update(node, NODE_KIND_PAIR, basic);
	return node;
}

node_t *
node_make_heritage(node_t *node, node_t *key, node_t *type)
{
	node_heritage_t *basic;
	if(!(basic = (node_heritage_t *)malloc(sizeof(node_heritage_t))))
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_heritage_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_heritage_t));
	basic->key = key;
	basic->type = type;
	
	node_update(node, NODE_KIND_HERITAGE, basic);
	return node;
}

node_t *
node_make_heritages(node_t *node, list_t *list)
{
	node_block_t *basic = (node_block_t *)malloc(sizeof(node_block_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_block_t));
	basic->list = list;
	
	node_update(node, NODE_KIND_HERITAGES, basic);
	return node;
}

node_t *
node_make_class(node_t *node, node_t *annotation, uint64_t flag, node_t *key, node_t *generics, node_t *heritages, node_t *block)
{
	node_class_t *basic;
	if(!(basic = (node_class_t *)malloc(sizeof(node_class_t))))
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_class_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_class_t));
	basic->flag = flag;
	basic->annotation = annotation;
	basic->key = key;
	basic->generics = generics;
	basic->heritages = heritages;
	basic->block = block;
	
	node_update(node, NODE_KIND_CLASS, basic);
	return node;
}

node_t *
node_make_annotation(node_t *node, node_t *key, node_t *arguments, node_t *next)
{
	node_note_t *basic = (node_note_t *)malloc(sizeof(node_note_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_note_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_note_t));
	basic->key = key;
	basic->arguments = arguments;
	basic->next = next;
	
	node_update(node, NODE_KIND_ANNOTATION, basic);
	return node;
}

node_t *
node_make_member(node_t *node, node_t *key, node_t *value){
	node_member_t *basic = (node_member_t *)malloc(sizeof(node_member_t));
	if(basic == NULL){
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_member_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_member_t));
	basic->key = key;
	basic->value = value;
	
	node_update(node, NODE_KIND_MEMBER, basic);
	return node;
}

node_t *
node_make_members(node_t *node, list_t *list)
{
	node_block_t *basic = (node_block_t *)malloc(sizeof(node_block_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_block_t));
	basic->list = list;
	
	node_update(node, NODE_KIND_MEMBERS, basic);
	return node;
}

node_t *
node_make_enum(node_t *node, node_t *annotation, uint64_t flag, node_t *key, node_t *block)
{
	node_enum_t *basic;
	if(!(basic = (node_enum_t *)malloc(sizeof(node_enum_t))))
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_enum_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_enum_t));
	basic->flag = flag;
	basic->annotation = annotation;
	basic->key = key;
	basic->block = block;
	
	node_update(node, NODE_KIND_ENUM, basic);
	return node;
}

node_t *
node_make_block(node_t *node, list_t *list)
{
	node_block_t *basic = (node_block_t *)malloc(sizeof(node_block_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_block_t));
	basic->list = list;
	
	node_update(node, NODE_KIND_BLOCK, basic);
	return node;
}

node_t *
node_make_body(node_t *node, list_t *list)
{
	node_block_t *basic = (node_block_t *)malloc(sizeof(node_block_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_block_t));
	basic->list = list;
	
	node_update(node, NODE_KIND_BODY, basic);
	return node;
}

node_t *
node_make_package(node_t *node, node_t *key, node_t *generics, node_t *address)
{
	node_package_t *basic = (node_package_t *)malloc(sizeof(node_package_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_package_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_package_t));
	basic->key = key;
	basic->generics = generics;
	basic->address = address;
	
	node_update(node, NODE_KIND_PACKAGE, basic);
	return node;
}

node_t *
node_make_packages(node_t *node, list_t *list)
{
	node_block_t *basic = (node_block_t *)malloc(sizeof(node_block_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_block_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_block_t));
	basic->list = list;
	
	node_update(node, NODE_KIND_PACKAGES, basic);
	return node;
}

node_t *
node_make_using(node_t *node, node_t *path, node_t *packages)
{
	node_using_t *basic = (node_using_t *)malloc(sizeof(node_using_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_using_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_using_t));
	basic->path = path;
	basic->packages = packages;
	
	node_update(node, NODE_KIND_USING, basic);
	return node;
}

node_t *
node_make_module(node_t *node, char *path, list_t *items)
{
	node_module_t *basic = (node_module_t *)malloc(sizeof(node_module_t));
	if(basic == NULL)
	{
		fprintf(stderr, "Unable to allocted a block of %zu bytes\n", sizeof(node_module_t));
		return NULL;
	}
	memset(basic, 0, sizeof(node_module_t));
	basic->items = items;
	basic->path = path;
	
	node_update(node, NODE_KIND_MODULE, basic);
	return node;
}
