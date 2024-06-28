#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../types/types.h"
#include "../container/queue.h"
#include "../token/position.h"
#include "../token/token.h"
#include "../memory.h"
#include "../error.h"
#include "node.h"

uint64_t node_counter = 0;

static const char *const symbols[] = {
	[NODE_KIND_ID] = "id",

	[NODE_KIND_NUMBER] = "number",
	[NODE_KIND_CHAR] = "char",
	[NODE_KIND_STRING] = "string",

	[NODE_KIND_TUPLE] = "tuple",
	[NODE_KIND_OBJECT] = "object",

	[NODE_KIND_PSEUDONYM] = "pseudonym",

	[NODE_KIND_NULL] = "null",
	[NODE_KIND_KINT] = "int",
	[NODE_KIND_KFLOAT] = "float",
	[NODE_KIND_KCHAR] = "char",
	[NODE_KIND_KSTRING] = "string",

	[NODE_KIND_TYPEOF] = "typeof",
	[NODE_KIND_SIZEOF] = "sizeof",
	[NODE_KIND_PARENTHESIS] = "parenthesis",

	[NODE_KIND_CALL] = "call",
	[NODE_KIND_ARRAY] = "array",
	[NODE_KIND_ATTRIBUTE] = "attribute",

	[NODE_KIND_TILDE] = "~",
	[NODE_KIND_NOT] = "!",
	[NODE_KIND_NEG] = "-",
	[NODE_KIND_POS] = "+",

	[NODE_KIND_POW] = "**",
	[NODE_KIND_EPI] = "\\",

	[NODE_KIND_MUL] = "*",
	[NODE_KIND_DIV] = "/",
	[NODE_KIND_MOD] = "%",

	[NODE_KIND_PLUS] = "+",
	[NODE_KIND_MINUS] = "-",

	[NODE_KIND_SHL] = "<<",
	[NODE_KIND_SHR] = ">>",

	[NODE_KIND_LT] = "<",
	[NODE_KIND_LE] = "<=",
	[NODE_KIND_GT] = ">",
	[NODE_KIND_GE] = ">=",

	[NODE_KIND_EQ] = "==",
	[NODE_KIND_NEQ] = "!=",

	[NODE_KIND_AND] = "&",
	[NODE_KIND_XOR] = "^",
	[NODE_KIND_OR] = "|",
	[NODE_KIND_LAND] = "&&",
	[NODE_KIND_LOR] = "||",

	[NODE_KIND_CONDITIONAL] = "?",

	[NODE_KIND_ASSIGN] = "=",
	[NODE_KIND_ADD_ASSIGN] = "+=",
	[NODE_KIND_SUB_ASSIGN] = "-=",
	[NODE_KIND_MUL_ASSIGN] = "*=",
	[NODE_KIND_DIV_ASSIGN] = "/=",
	[NODE_KIND_EPI_ASSIGN] = "\\=",
	[NODE_KIND_MOD_ASSIGN] = "%=",
	[NODE_KIND_AND_ASSIGN] = "&=",
	[NODE_KIND_OR_ASSIGN] = "|=",
	[NODE_KIND_SHL_ASSIGN] = "<<=",
	[NODE_KIND_SHR_ASSIGN] = ">>=",

	[NODE_KIND_IF] = "if",
	[NODE_KIND_FOR] = "for",
	[NODE_KIND_BREAK] = "break",
	[NODE_KIND_CONTINUE] = "continue",
	[NODE_KIND_CATCH] = "catch",
	[NODE_KIND_TRY] = "try",
	[NODE_KIND_RETURN] = "return",
	[NODE_KIND_THROW] = "throw",

	[NODE_KIND_VAR] = "var",
	[NODE_KIND_LAMBDA] = "lambda",
	[NODE_KIND_BODY] = "body",
	[NODE_KIND_FUN] = "fun",
	[NODE_KIND_CLASS] = "class",
	[NODE_KIND_PAIR] = "pair",
	[NODE_KIND_ENTITY] = "entity",
	[NODE_KIND_SET] = "set",
	[NODE_KIND_PROPERTY] = "property",
	[NODE_KIND_NOTE] = "note",
	[NODE_KIND_NOTES] = "notes",
	[NODE_KIND_ARGUMENT] = "argument",
	[NODE_KIND_ARGUMENTS] = "arguments",
	[NODE_KIND_PARAMETER] = "parameter",
	[NODE_KIND_PARAMETERS] = "parameters",
	[NODE_KIND_FIELD] = "field",
	[NODE_KIND_FIELDS] = "fields",
	[NODE_KIND_GENERIC] = "generic",
	[NODE_KIND_GENERICS] = "generics",
	[NODE_KIND_HERITAGE] = "heritage",
	[NODE_KIND_HERITAGES] = "heritages",
	[NODE_KIND_PACKAGE] = "package",
	[NODE_KIND_PACKAGES] = "packages",
	[NODE_KIND_USING] = "using",
	[NODE_KIND_MODULE] = "module"};

const char *
not_node_kind_as_string(not_node_t *node)
{
	return symbols[node->kind];
}

void not_node_destroy(not_node_t *node)
{
	if (!node)
	{
		return;
	}

	if (node->kind == NODE_KIND_MODULE)
	{
		not_node_block_t *basic = (not_node_block_t *)node->value;
		for (not_node_t *item = basic->items, *next = NULL; item != NULL; item = next)
		{
			next = item->next;
			not_node_destroy(item);
		}
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_USING)
	{
		not_node_using_t *basic = (not_node_using_t *)node->value;
		not_node_destroy(basic->packages);
		not_node_destroy(basic->path);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_PACKAGES)
	{
		not_node_block_t *basic = (not_node_block_t *)node->value;
		for (not_node_t *item = basic->items, *next = NULL; item != NULL; item = next)
		{
			next = item->next;
			not_node_destroy(item);
		}
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_PACKAGE)
	{
		not_node_package_t *basic = (not_node_package_t *)node->value;
		not_node_destroy(basic->key);
		not_node_destroy(basic->value);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_NOTES)
	{
		not_node_block_t *basic = (not_node_block_t *)node->value;
		for (not_node_t *item = basic->items, *next = NULL; item != NULL; item = next)
		{
			next = item->next;
			not_node_destroy(item);
		}
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_NOTE)
	{
		not_node_carrier_t *basic = (not_node_carrier_t *)node->value;
		not_node_destroy(basic->base);
		not_node_destroy(basic->data);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_BODY)
	{
		not_node_body_t *basic = (not_node_body_t *)node->value;
		for (not_node_t *item = basic->declaration, *next = NULL; item != NULL; item = next)
		{
			next = item->next;
			not_node_destroy(item);
		}
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_CLASS)
	{
		not_node_class_t *class = (not_node_class_t *)node->value;
		not_node_destroy(class->notes);
		not_node_destroy(class->key);
		not_node_destroy(class->generics);
		not_node_destroy(class->heritages);

		for (not_node_t *item = class->block, *next = NULL; item != NULL; item = next)
		{
			next = item->next;
			not_node_destroy(item);
		}
		not_memory_free(class);
	}
	else if (node->kind == NODE_KIND_FUN)
	{
		not_node_fun_t *fun = (not_node_fun_t *)node->value;
		not_node_destroy(fun->notes);
		not_node_destroy(fun->key);
		not_node_destroy(fun->generics);
		not_node_destroy(fun->parameters);
		not_node_destroy(fun->result);
		not_node_destroy(fun->body);
		not_memory_free(fun);
	}
	else if (node->kind == NODE_KIND_LAMBDA)
	{
		not_node_lambda_t *fun = (not_node_lambda_t *)node->value;
		not_node_destroy(fun->key);
		not_node_destroy(fun->generics);
		not_node_destroy(fun->parameters);
		not_node_destroy(fun->result);
		not_node_destroy(fun->body);
		not_memory_free(fun);
	}
	else if (node->kind == NODE_KIND_GENERICS)
	{
		not_node_block_t *basic = (not_node_block_t *)node->value;
		for (not_node_t *item = basic->items, *next = NULL; item != NULL; item = next)
		{
			next = item->next;
			not_node_destroy(item);
		}
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_HERITAGES)
	{
		not_node_block_t *basic = (not_node_block_t *)node->value;
		for (not_node_t *item = basic->items, *next = NULL; item != NULL; item = next)
		{
			next = item->next;
			not_node_destroy(item);
		}
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_PARAMETERS)
	{
		not_node_block_t *basic = (not_node_block_t *)node->value;
		for (not_node_t *item = basic->items, *next = NULL; item != NULL; item = next)
		{
			next = item->next;
			not_node_destroy(item);
		}
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_ARGUMENTS)
	{
		not_node_block_t *basic = (not_node_block_t *)node->value;
		for (not_node_t *item = basic->items, *next = NULL; item != NULL; item = next)
		{
			next = item->next;
			not_node_destroy(item);
		}
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_FIELDS)
	{
		not_node_block_t *basic = (not_node_block_t *)node->value;
		for (not_node_t *item = basic->items, *next = NULL; item != NULL; item = next)
		{
			next = item->next;
			not_node_destroy(item);
		}
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_OBJECT)
	{
		not_node_block_t *basic = (not_node_block_t *)node->value;
		for (not_node_t *item = basic->items, *next = NULL; item != NULL; item = next)
		{
			next = item->next;
			not_node_destroy(item);
		}
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_TUPLE)
	{
		not_node_block_t *basic = (not_node_block_t *)node->value;
		for (not_node_t *item = basic->items, *next = NULL; item != NULL; item = next)
		{
			next = item->next;
			not_node_destroy(item);
		}
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_PAIR)
	{
		not_node_pair_t *basic = (not_node_pair_t *)node->value;
		not_node_destroy(basic->key);
		not_node_destroy(basic->value);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_GENERIC)
	{
		not_node_generic_t *basic = (not_node_generic_t *)node->value;
		not_node_destroy(basic->key);
		not_node_destroy(basic->type);
		not_node_destroy(basic->value);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_HERITAGE)
	{
		not_node_heritage_t *basic = (not_node_heritage_t *)node->value;
		not_node_destroy(basic->key);
		not_node_destroy(basic->type);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_PROPERTY)
	{
		not_node_property_t *basic = (not_node_property_t *)node->value;
		not_node_destroy(basic->key);
		not_node_destroy(basic->type);
		not_node_destroy(basic->value);
		not_node_destroy(basic->notes);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_PARAMETER)
	{
		not_node_parameter_t *basic = (not_node_parameter_t *)node->value;
		not_node_destroy(basic->key);
		not_node_destroy(basic->type);
		not_node_destroy(basic->value);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_ARGUMENT)
	{
		not_node_argument_t *basic = (not_node_argument_t *)node->value;
		not_node_destroy(basic->key);
		not_node_destroy(basic->value);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_FIELD)
	{
		not_node_field_t *basic = (not_node_field_t *)node->value;
		not_node_destroy(basic->key);
		not_node_destroy(basic->value);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_FOR)
	{
		not_node_for_t *for1 = (not_node_for_t *)node->value;
		not_node_destroy(for1->key);
		not_node_destroy(for1->initializer);
		not_node_destroy(for1->condition);
		not_node_destroy(for1->incrementor);
		not_node_destroy(for1->body);
		not_memory_free(for1);
	}
	else if (node->kind == NODE_KIND_FORIN)
	{
		not_node_forin_t *for1 = (not_node_forin_t *)node->value;
		not_node_destroy(for1->key);
		not_node_destroy(for1->field);
		not_node_destroy(for1->value);
		not_node_destroy(for1->iterator);
		not_node_destroy(for1->body);
		not_memory_free(for1);
	}
	else if (node->kind == NODE_KIND_TRY)
	{
		not_node_try_t *try1 = (not_node_try_t *)node->value;
		not_node_destroy(try1->body);
		not_node_destroy(try1->catchs);
		not_memory_free(try1);
	}
	else if (node->kind == NODE_KIND_CATCH)
	{
		not_node_catch_t *catch1 = (not_node_catch_t *)node->value;
		not_node_destroy(catch1->body);
		not_node_destroy(catch1->next);
		not_node_destroy(catch1->parameters);
		not_memory_free(catch1);
	}
	else if (node->kind == NODE_KIND_IF)
	{
		not_node_if_t *if1 = (not_node_if_t *)node->value;
		not_node_destroy(if1->condition);
		not_node_destroy(if1->else_body);
		not_node_destroy(if1->then_body);
		not_memory_free(if1);
	}
	else if (node->kind == NODE_KIND_VAR)
	{
		not_node_var_t *var1 = (not_node_var_t *)node->value;
		not_node_destroy(var1->key);
		not_node_destroy(var1->type);
		not_node_destroy(var1->value);
		not_memory_free(var1);
	}
	else if (node->kind == NODE_KIND_SET)
	{
		not_node_block_t *basic = (not_node_block_t *)node->value;
		for (not_node_t *item = basic->items, *next = NULL; item != NULL; item = next)
		{
			next = item->next;
			not_node_destroy(item);
		}
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_ENTITY)
	{
		not_node_entity_t *entity = (not_node_entity_t *)node->value;
		not_node_destroy(entity->key);
		not_node_destroy(entity->type);
		not_node_destroy(entity->value);
		not_memory_free(entity);
	}
	else if (node->kind == NODE_KIND_BREAK)
	{
		not_node_unary_t *basic = (not_node_unary_t *)node->value;
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_CONTINUE)
	{
		not_node_unary_t *basic = (not_node_unary_t *)node->value;
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_THROW)
	{
		not_node_unary_t *basic = (not_node_unary_t *)node->value;
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_RETURN)
	{
		not_node_unary_t *basic = (not_node_unary_t *)node->value;
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_CONDITIONAL)
	{
		not_node_triple_t *basic = (not_node_triple_t *)node->value;
		not_node_destroy(basic->base);
		not_node_destroy(basic->left);
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_LOR)
	{
		not_node_binary_t *basic = (not_node_binary_t *)node->value;
		not_node_destroy(basic->left);
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_LAND)
	{
		not_node_binary_t *basic = (not_node_binary_t *)node->value;
		not_node_destroy(basic->left);
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_OR)
	{
		not_node_binary_t *basic = (not_node_binary_t *)node->value;
		not_node_destroy(basic->left);
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_XOR)
	{
		not_node_binary_t *basic = (not_node_binary_t *)node->value;
		not_node_destroy(basic->left);
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_AND)
	{
		not_node_binary_t *basic = (not_node_binary_t *)node->value;
		not_node_destroy(basic->left);
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_EQ)
	{
		not_node_binary_t *basic = (not_node_binary_t *)node->value;
		not_node_destroy(basic->left);
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_NEQ)
	{
		not_node_binary_t *basic = (not_node_binary_t *)node->value;
		not_node_destroy(basic->left);
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_SHL)
	{
		not_node_binary_t *basic = (not_node_binary_t *)node->value;
		not_node_destroy(basic->left);
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_SHR)
	{
		not_node_binary_t *basic = (not_node_binary_t *)node->value;
		not_node_destroy(basic->left);
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_LT)
	{
		not_node_binary_t *basic = (not_node_binary_t *)node->value;
		not_node_destroy(basic->left);
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_LE)
	{
		not_node_binary_t *basic = (not_node_binary_t *)node->value;
		not_node_destroy(basic->left);
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_GT)
	{
		not_node_binary_t *basic = (not_node_binary_t *)node->value;
		not_node_destroy(basic->left);
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_GE)
	{
		not_node_binary_t *basic = (not_node_binary_t *)node->value;
		not_node_destroy(basic->left);
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_MUL)
	{
		not_node_binary_t *basic = (not_node_binary_t *)node->value;
		not_node_destroy(basic->left);
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_DIV)
	{
		not_node_binary_t *basic = (not_node_binary_t *)node->value;
		not_node_destroy(basic->left);
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_MOD)
	{
		not_node_binary_t *basic = (not_node_binary_t *)node->value;
		not_node_destroy(basic->left);
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_EPI)
	{
		not_node_binary_t *basic = (not_node_binary_t *)node->value;
		not_node_destroy(basic->left);
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_POW)
	{
		not_node_binary_t *basic = (not_node_binary_t *)node->value;
		not_node_destroy(basic->left);
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_PLUS)
	{
		not_node_binary_t *basic = (not_node_binary_t *)node->value;
		not_node_destroy(basic->left);
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_MINUS)
	{
		not_node_binary_t *basic = (not_node_binary_t *)node->value;
		not_node_destroy(basic->left);
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_ASSIGN)
	{
		not_node_binary_t *basic = (not_node_binary_t *)node->value;
		not_node_destroy(basic->left);
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_OR_ASSIGN)
	{
		not_node_binary_t *basic = (not_node_binary_t *)node->value;
		not_node_destroy(basic->left);
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_AND_ASSIGN)
	{
		not_node_binary_t *basic = (not_node_binary_t *)node->value;
		not_node_destroy(basic->left);
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_SHL_ASSIGN)
	{
		not_node_binary_t *basic = (not_node_binary_t *)node->value;
		not_node_destroy(basic->left);
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_SHR_ASSIGN)
	{
		not_node_binary_t *basic = (not_node_binary_t *)node->value;
		not_node_destroy(basic->left);
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_MUL_ASSIGN)
	{
		not_node_binary_t *basic = (not_node_binary_t *)node->value;
		not_node_destroy(basic->left);
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_DIV_ASSIGN)
	{
		not_node_binary_t *basic = (not_node_binary_t *)node->value;
		not_node_destroy(basic->left);
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_MOD_ASSIGN)
	{
		not_node_binary_t *basic = (not_node_binary_t *)node->value;
		not_node_destroy(basic->left);
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_EPI_ASSIGN)
	{
		not_node_binary_t *basic = (not_node_binary_t *)node->value;
		not_node_destroy(basic->left);
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_POW_ASSIGN)
	{
		not_node_binary_t *basic = (not_node_binary_t *)node->value;
		not_node_destroy(basic->left);
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_SUB_ASSIGN)
	{
		not_node_binary_t *basic = (not_node_binary_t *)node->value;
		not_node_destroy(basic->left);
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_ADD_ASSIGN)
	{
		not_node_binary_t *basic = (not_node_binary_t *)node->value;
		not_node_destroy(basic->left);
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_INSTANCEOF)
	{
		not_node_binary_t *basic = (not_node_binary_t *)node->value;
		not_node_destroy(basic->left);
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_TILDE)
	{
		not_node_unary_t *basic = (not_node_unary_t *)node->value;
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_POS)
	{
		not_node_unary_t *basic = (not_node_unary_t *)node->value;
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_NEG)
	{
		not_node_unary_t *basic = (not_node_unary_t *)node->value;
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_NOT)
	{
		not_node_unary_t *basic = (not_node_unary_t *)node->value;
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_SIZEOF)
	{
		not_node_unary_t *basic = (not_node_unary_t *)node->value;
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_TYPEOF)
	{
		not_node_unary_t *basic = (not_node_unary_t *)node->value;
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_PARENTHESIS)
	{
		not_node_unary_t *basic = (not_node_unary_t *)node->value;
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_CALL)
	{
		not_node_carrier_t *basic = (not_node_carrier_t *)node->value;
		not_node_destroy(basic->base);
		not_node_destroy(basic->data);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_ARRAY)
	{
		not_node_carrier_t *basic = (not_node_carrier_t *)node->value;
		not_node_destroy(basic->base);
		not_node_destroy(basic->data);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_PSEUDONYM)
	{
		not_node_carrier_t *basic = (not_node_carrier_t *)node->value;
		not_node_destroy(basic->base);
		not_node_destroy(basic->data);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_ATTRIBUTE)
	{
		not_node_binary_t *basic = (not_node_binary_t *)node->value;
		not_node_destroy(basic->left);
		not_node_destroy(basic->right);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_ID)
	{
		not_node_basic_t *basic = (not_node_basic_t *)node->value;
		not_memory_free(basic->value);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_CHAR)
	{
		not_node_basic_t *basic = (not_node_basic_t *)node->value;
		not_memory_free(basic->value);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_NUMBER)
	{
		not_node_basic_t *basic = (not_node_basic_t *)node->value;
		not_memory_free(basic->value);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_STRING)
	{
		not_node_basic_t *basic = (not_node_basic_t *)node->value;
		not_memory_free(basic->value);
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_NULL)
	{
		not_node_basic_t *basic = (not_node_basic_t *)node->value;
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_UNDEFINED)
	{
		not_node_basic_t *basic = (not_node_basic_t *)node->value;
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_NAN)
	{
		not_node_basic_t *basic = (not_node_basic_t *)node->value;
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_KCHAR)
	{
		not_node_basic_t *basic = (not_node_basic_t *)node->value;
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_KFLOAT)
	{
		not_node_basic_t *basic = (not_node_basic_t *)node->value;
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_KINT)
	{
		not_node_basic_t *basic = (not_node_basic_t *)node->value;
		not_memory_free(basic);
	}
	else if (node->kind == NODE_KIND_KSTRING)
	{
		not_node_basic_t *basic = (not_node_basic_t *)node->value;
		not_memory_free(basic);
	}

	// printf("node = %d\n", node->kind);
	not_memory_free(node);
}

not_node_t *
not_node_create(not_node_t *parent, not_position_t position)
{
	not_node_t *node = (not_node_t *)not_memory_calloc(1, sizeof(not_node_t));
	if (!node)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	node->id = node_counter++;
	node->position = position;
	node->parent = parent;
	node->kind = NODE_KIND_RAW;

	node->next = node->previous = NULL;

	return node;
}

static void
not_node_update(not_node_t *node, int32_t kind, void *value)
{
	node->value = value;
	node->kind = kind;
}

void not_node_remove(not_node_t *node)
{
	free(node);
}

not_node_t *
not_node_make_id(not_node_t *node, char *value)
{
	not_node_basic_t *basic = (not_node_basic_t *)not_memory_calloc(1, sizeof(not_node_basic_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	char *str = not_memory_calloc(strlen(value) + 1, sizeof(char));
	if (!str)
	{
		not_error_no_memory();
		not_memory_free(str);
		return NOT_PTR_ERROR;
	}

	strcpy(str, value);

	basic->value = str;

	not_node_update(node, NODE_KIND_ID, basic);
	return node;
}

not_node_t *
not_node_make_number(not_node_t *node, char *value)
{
	not_node_basic_t *basic = (not_node_basic_t *)not_memory_calloc(1, sizeof(not_node_basic_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	char *str = not_memory_calloc(strlen(value) + 1, sizeof(char));
	if (!str)
	{
		not_error_no_memory();
		not_memory_free(str);
		return NOT_PTR_ERROR;
	}

	strcpy(str, value);

	basic->value = str;

	not_node_update(node, NODE_KIND_NUMBER, basic);
	return node;
}

not_node_t *
not_node_make_char(not_node_t *node, char *value)
{
	not_node_basic_t *basic = (not_node_basic_t *)not_memory_calloc(1, sizeof(not_node_basic_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	char *str = not_memory_calloc(strlen(value) + 1, sizeof(char));
	if (!str)
	{
		not_error_no_memory();
		not_memory_free(str);
		return NOT_PTR_ERROR;
	}

	strcpy(str, value);

	basic->value = str;

	not_node_update(node, NODE_KIND_CHAR, basic);
	return node;
}

not_node_t *
not_node_make_string(not_node_t *node, char *value)
{
	not_node_basic_t *basic = (not_node_basic_t *)not_memory_calloc(1, sizeof(not_node_basic_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	char *str = not_memory_calloc(strlen(value) + 1, sizeof(char));
	if (!str)
	{
		not_error_no_memory();
		not_memory_free(str);
		return NOT_PTR_ERROR;
	}

	strcpy(str, value);

	basic->value = str;

	not_node_update(node, NODE_KIND_STRING, basic);
	return node;
}

not_node_t *
not_node_make_null(not_node_t *node)
{
	not_node_basic_t *basic = (not_node_basic_t *)not_memory_calloc(1, sizeof(not_node_basic_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->value = NULL;

	not_node_update(node, NODE_KIND_NULL, basic);
	return node;
}

not_node_t *
not_node_make_undefined(not_node_t *node)
{
	not_node_basic_t *basic = (not_node_basic_t *)not_memory_calloc(1, sizeof(not_node_basic_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->value = NULL;

	not_node_update(node, NODE_KIND_UNDEFINED, basic);
	return node;
}

not_node_t *
not_node_make_nan(not_node_t *node)
{
	not_node_basic_t *basic = (not_node_basic_t *)not_memory_calloc(1, sizeof(not_node_basic_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->value = NULL;

	not_node_update(node, NODE_KIND_NAN, basic);
	return node;
}

not_node_t *
not_node_make_kint(not_node_t *node)
{
	not_node_basic_t *basic = (not_node_basic_t *)not_memory_calloc(1, sizeof(not_node_basic_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->value = NULL;

	not_node_update(node, NODE_KIND_KINT, basic);

	return node;
}

not_node_t *
not_node_make_kfloat(not_node_t *node)
{
	not_node_basic_t *basic = (not_node_basic_t *)not_memory_calloc(1, sizeof(not_node_basic_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->value = NULL;

	not_node_update(node, NODE_KIND_KFLOAT, basic);

	return node;
}

not_node_t *
not_node_make_kchar(not_node_t *node)
{
	not_node_basic_t *basic = (not_node_basic_t *)not_memory_calloc(1, sizeof(not_node_basic_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->value = NULL;

	not_node_update(node, NODE_KIND_KCHAR, basic);

	return node;
}

not_node_t *
not_node_make_kstring(not_node_t *node)
{
	not_node_basic_t *basic = (not_node_basic_t *)not_memory_calloc(1, sizeof(not_node_basic_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->value = NULL;

	not_node_update(node, NODE_KIND_KSTRING, basic);

	return node;
}

not_node_t *
not_node_make_tuple(not_node_t *node, not_node_t *items)
{
	not_node_block_t *basic = (not_node_block_t *)not_memory_calloc(1, sizeof(not_node_block_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->items = items;

	not_node_update(node, NODE_KIND_TUPLE, basic);
	return node;
}

not_node_t *
not_node_make_object(not_node_t *node, not_node_t *items)
{
	not_node_block_t *basic = (not_node_block_t *)not_memory_calloc(1, sizeof(not_node_block_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->items = items;

	not_node_update(node, NODE_KIND_OBJECT, basic);
	return node;
}

not_node_t *
not_node_make_pseudonym(not_node_t *node, not_node_t *base, not_node_t *concepts)
{
	not_node_carrier_t *basic = (not_node_carrier_t *)not_memory_calloc(1, sizeof(not_node_carrier_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->base = base;
	basic->data = concepts;

	not_node_update(node, NODE_KIND_PSEUDONYM, basic);
	return node;
}

not_node_t *
not_node_make_typeof(not_node_t *node, not_node_t *right)
{
	not_node_unary_t *basic = (not_node_unary_t *)not_memory_calloc(1, sizeof(not_node_unary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->right = right;

	not_node_update(node, NODE_KIND_TYPEOF, basic);
	return node;
}

not_node_t *
not_node_make_sizeof(not_node_t *node, not_node_t *right)
{
	not_node_unary_t *basic = (not_node_unary_t *)not_memory_calloc(1, sizeof(not_node_unary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->right = right;

	not_node_update(node, NODE_KIND_SIZEOF, basic);
	return node;
}

not_node_t *
not_node_make_parenthesis(not_node_t *node, not_node_t *value)
{
	not_node_unary_t *basic = (not_node_unary_t *)not_memory_calloc(1, sizeof(not_node_unary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->right = value;

	not_node_update(node, NODE_KIND_PARENTHESIS, basic);
	return node;
}

not_node_t *
not_node_make_call(not_node_t *node, not_node_t *base, not_node_t *arguments)
{
	not_node_carrier_t *basic = (not_node_carrier_t *)not_memory_calloc(1, sizeof(not_node_carrier_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->base = base;
	basic->data = arguments;

	not_node_update(node, NODE_KIND_CALL, basic);
	return node;
}

not_node_t *
not_node_make_array(not_node_t *node, not_node_t *base, not_node_t *arguments)
{
	not_node_carrier_t *basic = (not_node_carrier_t *)not_memory_calloc(1, sizeof(not_node_carrier_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->base = base;
	basic->data = arguments;

	not_node_update(node, NODE_KIND_ARRAY, basic);
	return node;
}

not_node_t *
not_node_make_attribute(not_node_t *node, not_node_t *left, not_node_t *right)
{
	not_node_binary_t *basic = (not_node_binary_t *)not_memory_calloc(1, sizeof(not_node_binary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->left = left;
	basic->right = right;

	not_node_update(node, NODE_KIND_ATTRIBUTE, basic);
	return node;
}

not_node_t *
not_node_make_tilde(not_node_t *node, not_node_t *right)
{
	not_node_unary_t *basic = (not_node_unary_t *)not_memory_calloc(1, sizeof(not_node_unary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->right = right;

	not_node_update(node, NODE_KIND_TILDE, basic);
	return node;
}

not_node_t *
not_node_make_not(not_node_t *node, not_node_t *right)
{
	not_node_unary_t *basic = (not_node_unary_t *)not_memory_calloc(1, sizeof(not_node_unary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->right = right;

	not_node_update(node, NODE_KIND_NOT, basic);
	return node;
}

not_node_t *
not_node_make_neg(not_node_t *node, not_node_t *right)
{
	not_node_unary_t *basic = (not_node_unary_t *)not_memory_calloc(1, sizeof(not_node_unary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->right = right;

	not_node_update(node, NODE_KIND_NEG, basic);
	return node;
}

not_node_t *
not_node_make_pos(not_node_t *node, not_node_t *right)
{
	not_node_unary_t *basic = (not_node_unary_t *)not_memory_calloc(1, sizeof(not_node_unary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->right = right;

	not_node_update(node, NODE_KIND_POS, basic);
	return node;
}

not_node_t *
not_node_make_pow(not_node_t *node, not_node_t *left, not_node_t *right)
{
	not_node_binary_t *basic = (not_node_binary_t *)not_memory_calloc(1, sizeof(not_node_binary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->left = left;
	basic->right = right;

	not_node_update(node, NODE_KIND_POW, basic);
	return node;
}

not_node_t *
not_node_make_epi(not_node_t *node, not_node_t *left, not_node_t *right)
{
	not_node_binary_t *basic = (not_node_binary_t *)not_memory_calloc(1, sizeof(not_node_binary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->left = left;
	basic->right = right;

	not_node_update(node, NODE_KIND_EPI, basic);
	return node;
}

not_node_t *
not_node_make_mul(not_node_t *node, not_node_t *left, not_node_t *right)
{
	not_node_binary_t *basic = (not_node_binary_t *)not_memory_calloc(1, sizeof(not_node_binary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->left = left;
	basic->right = right;

	not_node_update(node, NODE_KIND_MUL, basic);
	return node;
}

not_node_t *
not_node_make_div(not_node_t *node, not_node_t *left, not_node_t *right)
{
	not_node_binary_t *basic = (not_node_binary_t *)not_memory_calloc(1, sizeof(not_node_binary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->left = left;
	basic->right = right;

	not_node_update(node, NODE_KIND_DIV, basic);
	return node;
}

not_node_t *
not_node_make_mod(not_node_t *node, not_node_t *left, not_node_t *right)
{
	not_node_binary_t *basic = (not_node_binary_t *)not_memory_calloc(1, sizeof(not_node_binary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->left = left;
	basic->right = right;

	not_node_update(node, NODE_KIND_MOD, basic);
	return node;
}

not_node_t *
not_node_make_plus(not_node_t *node, not_node_t *left, not_node_t *right)
{
	not_node_binary_t *basic = (not_node_binary_t *)not_memory_calloc(1, sizeof(not_node_binary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->left = left;
	basic->right = right;

	not_node_update(node, NODE_KIND_PLUS, basic);
	return node;
}

not_node_t *
not_node_make_minus(not_node_t *node, not_node_t *left, not_node_t *right)
{
	not_node_binary_t *basic = (not_node_binary_t *)not_memory_calloc(1, sizeof(not_node_binary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->left = left;
	basic->right = right;

	not_node_update(node, NODE_KIND_MINUS, basic);
	return node;
}

not_node_t *
not_node_make_shl(not_node_t *node, not_node_t *left, not_node_t *right)
{
	not_node_binary_t *basic = (not_node_binary_t *)not_memory_calloc(1, sizeof(not_node_binary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->left = left;
	basic->right = right;

	not_node_update(node, NODE_KIND_SHL, basic);
	return node;
}

not_node_t *
not_node_make_shr(not_node_t *node, not_node_t *left, not_node_t *right)
{
	not_node_binary_t *basic = (not_node_binary_t *)not_memory_calloc(1, sizeof(not_node_binary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->left = left;
	basic->right = right;

	not_node_update(node, NODE_KIND_SHR, basic);
	return node;
}

not_node_t *
not_node_make_lt(not_node_t *node, not_node_t *left, not_node_t *right)
{
	not_node_binary_t *basic = (not_node_binary_t *)not_memory_calloc(1, sizeof(not_node_binary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->left = left;
	basic->right = right;

	not_node_update(node, NODE_KIND_LT, basic);
	return node;
}

not_node_t *
not_node_make_le(not_node_t *node, not_node_t *left, not_node_t *right)
{
	not_node_binary_t *basic = (not_node_binary_t *)not_memory_calloc(1, sizeof(not_node_binary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->left = left;
	basic->right = right;

	not_node_update(node, NODE_KIND_LE, basic);
	return node;
}

not_node_t *
not_node_make_gt(not_node_t *node, not_node_t *left, not_node_t *right)
{
	not_node_binary_t *basic = (not_node_binary_t *)not_memory_calloc(1, sizeof(not_node_binary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->left = left;
	basic->right = right;

	not_node_update(node, NODE_KIND_GT, basic);
	return node;
}

not_node_t *
not_node_make_ge(not_node_t *node, not_node_t *left, not_node_t *right)
{
	not_node_binary_t *basic = (not_node_binary_t *)not_memory_calloc(1, sizeof(not_node_binary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->left = left;
	basic->right = right;

	not_node_update(node, NODE_KIND_GE, basic);
	return node;
}

not_node_t *
not_node_make_eq(not_node_t *node, not_node_t *left, not_node_t *right)
{
	not_node_binary_t *basic = (not_node_binary_t *)not_memory_calloc(1, sizeof(not_node_binary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->left = left;
	basic->right = right;

	not_node_update(node, NODE_KIND_EQ, basic);
	return node;
}

not_node_t *
not_node_make_neq(not_node_t *node, not_node_t *left, not_node_t *right)
{
	not_node_binary_t *basic = (not_node_binary_t *)not_memory_calloc(1, sizeof(not_node_binary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->left = left;
	basic->right = right;

	not_node_update(node, NODE_KIND_NEQ, basic);
	return node;
}

not_node_t *
not_node_make_and(not_node_t *node, not_node_t *left, not_node_t *right)
{
	not_node_binary_t *basic = (not_node_binary_t *)not_memory_calloc(1, sizeof(not_node_binary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->left = left;
	basic->right = right;

	not_node_update(node, NODE_KIND_AND, basic);
	return node;
}

not_node_t *
not_node_make_xor(not_node_t *node, not_node_t *left, not_node_t *right)
{
	not_node_binary_t *basic = (not_node_binary_t *)not_memory_calloc(1, sizeof(not_node_binary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->left = left;
	basic->right = right;

	not_node_update(node, NODE_KIND_XOR, basic);
	return node;
}

not_node_t *
not_node_make_or(not_node_t *node, not_node_t *left, not_node_t *right)
{
	not_node_binary_t *basic = (not_node_binary_t *)not_memory_calloc(1, sizeof(not_node_binary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->left = left;
	basic->right = right;

	not_node_update(node, NODE_KIND_OR, basic);
	return node;
}

not_node_t *
not_node_make_land(not_node_t *node, not_node_t *left, not_node_t *right)
{
	not_node_binary_t *basic = (not_node_binary_t *)not_memory_calloc(1, sizeof(not_node_binary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->left = left;
	basic->right = right;

	not_node_update(node, NODE_KIND_LAND, basic);
	return node;
}

not_node_t *
not_node_make_lor(not_node_t *node, not_node_t *left, not_node_t *right)
{
	not_node_binary_t *basic = (not_node_binary_t *)not_memory_calloc(1, sizeof(not_node_binary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->left = left;
	basic->right = right;

	not_node_update(node, NODE_KIND_LOR, basic);
	return node;
}

not_node_t *
not_node_make_instanceof(not_node_t *node, not_node_t *left, not_node_t *right)
{
	not_node_binary_t *basic = (not_node_binary_t *)not_memory_calloc(1, sizeof(not_node_binary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->left = left;
	basic->right = right;

	not_node_update(node, NODE_KIND_INSTANCEOF, basic);
	return node;
}

not_node_t *
not_node_make_conditional(not_node_t *node, not_node_t *base, not_node_t *left, not_node_t *right)
{
	not_node_triple_t *basic = (not_node_triple_t *)not_memory_calloc(1, sizeof(not_node_triple_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->base = base;
	basic->left = left;
	basic->right = right;

	not_node_update(node, NODE_KIND_CONDITIONAL, basic);
	return node;
}

not_node_t *
not_node_make_assign(not_node_t *node, not_node_t *left, not_node_t *right)
{
	not_node_binary_t *basic = (not_node_binary_t *)not_memory_calloc(1, sizeof(not_node_binary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->left = left;
	basic->right = right;

	not_node_update(node, NODE_KIND_ASSIGN, basic);
	return node;
}

not_node_t *
not_node_make_add_assign(not_node_t *node, not_node_t *left, not_node_t *right)
{
	not_node_binary_t *basic = (not_node_binary_t *)not_memory_calloc(1, sizeof(not_node_binary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->left = left;
	basic->right = right;

	not_node_update(node, NODE_KIND_ADD_ASSIGN, basic);
	return node;
}

not_node_t *
not_node_make_sub_assign(not_node_t *node, not_node_t *left, not_node_t *right)
{
	not_node_binary_t *basic = (not_node_binary_t *)not_memory_calloc(1, sizeof(not_node_binary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->left = left;
	basic->right = right;

	not_node_update(node, NODE_KIND_SUB_ASSIGN, basic);
	return node;
}

not_node_t *
not_node_make_div_assign(not_node_t *node, not_node_t *left, not_node_t *right)
{
	not_node_binary_t *basic = (not_node_binary_t *)not_memory_calloc(1, sizeof(not_node_binary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->left = left;
	basic->right = right;

	not_node_update(node, NODE_KIND_DIV_ASSIGN, basic);
	return node;
}

not_node_t *
not_node_make_epi_assign(not_node_t *node, not_node_t *left, not_node_t *right)
{
	not_node_binary_t *basic = (not_node_binary_t *)not_memory_calloc(1, sizeof(not_node_binary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->left = left;
	basic->right = right;

	not_node_update(node, NODE_KIND_EPI_ASSIGN, basic);
	return node;
}

not_node_t *
not_node_make_mul_assign(not_node_t *node, not_node_t *left, not_node_t *right)
{
	not_node_binary_t *basic = (not_node_binary_t *)not_memory_calloc(1, sizeof(not_node_binary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->left = left;
	basic->right = right;

	not_node_update(node, NODE_KIND_MUL_ASSIGN, basic);
	return node;
}

not_node_t *
not_node_make_mod_assign(not_node_t *node, not_node_t *left, not_node_t *right)
{
	not_node_binary_t *basic = (not_node_binary_t *)not_memory_calloc(1, sizeof(not_node_binary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->left = left;
	basic->right = right;

	not_node_update(node, NODE_KIND_MOD_ASSIGN, basic);
	return node;
}

not_node_t *
not_node_make_pow_assign(not_node_t *node, not_node_t *left, not_node_t *right)
{
	not_node_binary_t *basic = (not_node_binary_t *)not_memory_calloc(1, sizeof(not_node_binary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->left = left;
	basic->right = right;

	not_node_update(node, NODE_KIND_POW_ASSIGN, basic);
	return node;
}

not_node_t *
not_node_make_and_assign(not_node_t *node, not_node_t *left, not_node_t *right)
{
	not_node_binary_t *basic = (not_node_binary_t *)not_memory_calloc(1, sizeof(not_node_binary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->left = left;
	basic->right = right;

	not_node_update(node, NODE_KIND_AND_ASSIGN, basic);
	return node;
}

not_node_t *
not_node_make_or_assign(not_node_t *node, not_node_t *left, not_node_t *right)
{
	not_node_binary_t *basic = (not_node_binary_t *)not_memory_calloc(1, sizeof(not_node_binary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->left = left;
	basic->right = right;

	not_node_update(node, NODE_KIND_OR_ASSIGN, basic);
	return node;
}

not_node_t *
not_node_make_shl_assign(not_node_t *node, not_node_t *left, not_node_t *right)
{
	not_node_binary_t *basic = (not_node_binary_t *)not_memory_calloc(1, sizeof(not_node_binary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->left = left;
	basic->right = right;

	not_node_update(node, NODE_KIND_SHL_ASSIGN, basic);
	return node;
}

not_node_t *
not_node_make_shr_assign(not_node_t *node, not_node_t *left, not_node_t *right)
{
	not_node_binary_t *basic = (not_node_binary_t *)not_memory_calloc(1, sizeof(not_node_binary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->left = left;
	basic->right = right;

	not_node_update(node, NODE_KIND_SHR_ASSIGN, basic);
	return node;
}

not_node_t *
not_node_make_throw(not_node_t *node, not_node_t *value)
{
	not_node_unary_t *basic = (not_node_unary_t *)not_memory_calloc(1, sizeof(not_node_unary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->right = value;

	not_node_update(node, NODE_KIND_THROW, basic);
	return node;
}

not_node_t *
not_node_make_break(not_node_t *node, not_node_t *expression)
{
	not_node_unary_t *basic = (not_node_unary_t *)not_memory_calloc(1, sizeof(not_node_unary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->right = expression;

	not_node_update(node, NODE_KIND_BREAK, basic);
	return node;
}

not_node_t *
not_node_make_continue(not_node_t *node, not_node_t *expression)
{
	not_node_unary_t *basic = (not_node_unary_t *)not_memory_calloc(1, sizeof(not_node_unary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->right = expression;

	not_node_update(node, NODE_KIND_CONTINUE, basic);
	return node;
}

not_node_t *
not_node_make_return(not_node_t *node, not_node_t *expression)
{
	not_node_unary_t *basic = (not_node_unary_t *)not_memory_calloc(1, sizeof(not_node_unary_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->right = expression;

	not_node_update(node, NODE_KIND_RETURN, basic);
	return node;
}

not_node_t *
not_node_make_if(not_node_t *node, not_node_t *condition, not_node_t *then_body, not_node_t *else_body)
{
	not_node_if_t *basic;
	if (!(basic = (not_node_if_t *)not_memory_calloc(1, sizeof(not_node_if_t))))
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->condition = condition;
	basic->then_body = then_body;
	basic->else_body = else_body;

	not_node_update(node, NODE_KIND_IF, basic);
	return node;
}

not_node_t *
not_node_make_for(not_node_t *node, not_node_t *key, not_node_t *initializer, not_node_t *condition, not_node_t *incrementor, not_node_t *body)
{
	not_node_for_t *basic = (not_node_for_t *)not_memory_calloc(1, sizeof(not_node_for_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->key = key;
	basic->initializer = initializer;
	basic->condition = condition;
	basic->incrementor = incrementor;
	basic->body = body;

	not_node_update(node, NODE_KIND_FOR, basic);
	return node;
}

not_node_t *
not_node_make_forin(not_node_t *node, not_node_t *key, not_node_t *field, not_node_t *value, not_node_t *iterator, not_node_t *body)
{
	not_node_forin_t *basic = (not_node_forin_t *)not_memory_calloc(1, sizeof(not_node_forin_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->key = key;
	basic->field = field;
	basic->value = value;
	basic->iterator = iterator;
	basic->body = body;

	not_node_update(node, NODE_KIND_FORIN, basic);
	return node;
}

not_node_t *
not_node_make_catch(not_node_t *node, not_node_t *parameters, not_node_t *body)
{
	not_node_catch_t *basic = (not_node_catch_t *)not_memory_calloc(1, sizeof(not_node_catch_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->parameters = parameters;
	basic->body = body;

	not_node_update(node, NODE_KIND_CATCH, basic);
	return node;
}

not_node_t *
not_node_make_try(not_node_t *node, not_node_t *body, not_node_t *catchs)
{
	not_node_try_t *basic;
	if (!(basic = (not_node_try_t *)not_memory_calloc(1, sizeof(not_node_try_t))))
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->body = body;
	basic->catchs = catchs;

	not_node_update(node, NODE_KIND_TRY, basic);
	return node;
}

not_node_t *
not_node_make_var(not_node_t *node, uint64_t flag, not_node_t *key, not_node_t *type, not_node_t *value)
{
	not_node_var_t *basic = (not_node_var_t *)not_memory_calloc(1, sizeof(not_node_var_t));

	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->flag = flag;
	basic->key = key;
	basic->type = type;
	basic->value = value;

	not_node_update(node, NODE_KIND_VAR, basic);
	return node;
}

not_node_t *
not_node_make_argument(not_node_t *node, not_node_t *key, not_node_t *value)
{
	not_node_argument_t *basic = (not_node_argument_t *)not_memory_calloc(1, sizeof(not_node_argument_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->key = key;
	basic->value = value;

	not_node_update(node, NODE_KIND_ARGUMENT, basic);
	return node;
}

not_node_t *
not_node_make_arguments(not_node_t *node, not_node_t *items)
{
	not_node_block_t *basic = (not_node_block_t *)not_memory_calloc(1, sizeof(not_node_block_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->items = items;

	not_node_update(node, NODE_KIND_ARGUMENTS, basic);
	return node;
}

not_node_t *
not_node_make_parameter(not_node_t *node, uint64_t flag, not_node_t *key, not_node_t *type, not_node_t *value)
{
	not_node_parameter_t *basic = (not_node_parameter_t *)not_memory_calloc(1, sizeof(not_node_parameter_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->flag = flag;
	basic->key = key;
	basic->type = type;
	basic->value = value;

	not_node_update(node, NODE_KIND_PARAMETER, basic);
	return node;
}

not_node_t *
not_node_make_parameters(not_node_t *node, not_node_t *items)
{
	not_node_block_t *basic = (not_node_block_t *)not_memory_calloc(1, sizeof(not_node_block_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->items = items;

	not_node_update(node, NODE_KIND_PARAMETERS, basic);
	return node;
}

not_node_t *
not_node_make_field(not_node_t *node, not_node_t *key, not_node_t *value)
{
	not_node_field_t *basic = (not_node_field_t *)not_memory_calloc(1, sizeof(not_node_field_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->key = key;
	basic->value = value;

	not_node_update(node, NODE_KIND_FIELD, basic);
	return node;
}

not_node_t *
not_node_make_fields(not_node_t *node, not_node_t *items)
{
	not_node_block_t *basic = (not_node_block_t *)not_memory_calloc(1, sizeof(not_node_block_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->items = items;

	not_node_update(node, NODE_KIND_FIELDS, basic);
	return node;
}

not_node_t *
not_node_make_generic(not_node_t *node, not_node_t *key, not_node_t *type, not_node_t *value)
{
	not_node_generic_t *basic = (not_node_generic_t *)not_memory_calloc(1, sizeof(not_node_generic_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->key = key;
	basic->type = type;
	basic->value = value;

	not_node_update(node, NODE_KIND_GENERIC, basic);
	return node;
}

not_node_t *
not_node_make_generics(not_node_t *node, not_node_t *items)
{
	not_node_block_t *basic = (not_node_block_t *)not_memory_calloc(1, sizeof(not_node_block_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->items = items;

	not_node_update(node, NODE_KIND_GENERICS, basic);
	return node;
}

not_node_t *
not_node_make_func(not_node_t *node, not_node_t *notes, uint64_t flag, not_node_t *key, not_node_t *generics, not_node_t *parameters, not_node_t *result, not_node_t *body)
{
	not_node_fun_t *basic = (not_node_fun_t *)not_memory_calloc(1, sizeof(not_node_fun_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->flag = flag;
	basic->notes = notes;
	basic->key = key;
	basic->generics = generics;
	basic->parameters = parameters;
	basic->result = result;
	basic->body = body;

	not_node_update(node, NODE_KIND_FUN, basic);
	return node;
}

not_node_t *
not_node_make_lambda(not_node_t *node, not_node_t *key, not_node_t *generics, not_node_t *parameters, not_node_t *body, not_node_t *result)
{
	not_node_lambda_t *basic = (not_node_lambda_t *)not_memory_calloc(1, sizeof(not_node_lambda_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->key = key;
	basic->generics = generics;
	basic->parameters = parameters;
	basic->body = body;
	basic->result = result;

	not_node_update(node, NODE_KIND_LAMBDA, basic);
	return node;
}

not_node_t *
not_node_make_property(not_node_t *node, not_node_t *notes, uint64_t flag, not_node_t *key, not_node_t *type, not_node_t *value)
{
	not_node_property_t *basic = (not_node_property_t *)not_memory_calloc(1, sizeof(not_node_property_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->flag = flag;
	basic->notes = notes;
	basic->key = key;
	basic->type = type;
	basic->value = value;

	not_node_update(node, NODE_KIND_PROPERTY, basic);
	return node;
}

not_node_t *
not_node_make_entity(not_node_t *node, uint64_t flag, not_node_t *key, not_node_t *type, not_node_t *value)
{
	not_node_entity_t *basic = (not_node_entity_t *)not_memory_calloc(1, sizeof(not_node_entity_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->flag = flag;
	basic->key = key;
	basic->type = type;
	basic->value = value;

	not_node_update(node, NODE_KIND_ENTITY, basic);
	return node;
}

not_node_t *
not_node_make_set(not_node_t *node, not_node_t *items)
{
	not_node_block_t *basic = (not_node_block_t *)not_memory_calloc(1, sizeof(not_node_block_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->items = items;

	not_node_update(node, NODE_KIND_SET, basic);
	return node;
}

not_node_t *
not_node_make_pair(not_node_t *node, not_node_t *key, not_node_t *value)
{
	not_node_pair_t *basic = (not_node_pair_t *)not_memory_calloc(1, sizeof(not_node_pair_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->key = key;
	basic->value = value;

	not_node_update(node, NODE_KIND_PAIR, basic);
	return node;
}

not_node_t *
not_node_make_heritage(not_node_t *node, not_node_t *key, not_node_t *type)
{
	not_node_heritage_t *basic = (not_node_heritage_t *)not_memory_calloc(1, sizeof(not_node_heritage_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->key = key;
	basic->type = type;

	not_node_update(node, NODE_KIND_HERITAGE, basic);
	return node;
}

not_node_t *
not_node_make_heritages(not_node_t *node, not_node_t *items)
{
	not_node_block_t *basic = (not_node_block_t *)not_memory_calloc(1, sizeof(not_node_block_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->items = items;

	not_node_update(node, NODE_KIND_HERITAGES, basic);
	return node;
}

not_node_t *
not_node_make_class(not_node_t *node, not_node_t *notes, uint64_t flag, not_node_t *key, not_node_t *generics, not_node_t *heritages, not_node_t *block)
{
	not_node_class_t *basic = (not_node_class_t *)not_memory_calloc(1, sizeof(not_node_class_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->flag = flag;
	basic->notes = notes;
	basic->key = key;
	basic->generics = generics;
	basic->heritages = heritages;
	basic->block = block;

	not_node_update(node, NODE_KIND_CLASS, basic);
	return node;
}

not_node_t *
not_node_make_note(not_node_t *node, not_node_t *key, not_node_t *arguments)
{
	not_node_carrier_t *basic = (not_node_carrier_t *)not_memory_calloc(1, sizeof(not_node_carrier_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->base = key;
	basic->data = arguments;

	not_node_update(node, NODE_KIND_NOTE, basic);
	return node;
}

not_node_t *
not_node_make_notes(not_node_t *node, not_node_t *items)
{
	not_node_block_t *basic = (not_node_block_t *)not_memory_calloc(1, sizeof(not_node_block_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->items = items;

	not_node_update(node, NODE_KIND_NOTES, basic);
	return node;
}

not_node_t *
not_node_make_package(not_node_t *node, not_node_t *key, not_node_t *value)
{
	not_node_package_t *basic = (not_node_package_t *)not_memory_calloc(1, sizeof(not_node_package_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->key = key;
	basic->value = value;

	not_node_update(node, NODE_KIND_PACKAGE, basic);
	return node;
}

not_node_t *
not_node_make_packages(not_node_t *node, not_node_t *items)
{
	not_node_block_t *basic = (not_node_block_t *)not_memory_calloc(1, sizeof(not_node_block_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->items = items;

	not_node_update(node, NODE_KIND_PACKAGES, basic);
	return node;
}

not_node_t *
not_node_make_using(not_node_t *node, char *base, not_node_t *path, not_node_t *packages)
{
	not_node_using_t *basic = (not_node_using_t *)not_memory_calloc(1, sizeof(not_node_using_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->base = base;
	basic->path = path;
	basic->packages = packages;

	not_node_update(node, NODE_KIND_USING, basic);
	return node;
}

not_node_t *
not_node_make_body(not_node_t *node, not_node_t *declaration)
{
	not_node_body_t *basic = (not_node_body_t *)not_memory_calloc(1, sizeof(not_node_body_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->declaration = declaration;

	not_node_update(node, NODE_KIND_BODY, basic);
	return node;
}

not_node_t *
not_node_make_module(not_node_t *node, not_node_t *items)
{
	not_node_block_t *basic = (not_node_block_t *)not_memory_calloc(1, sizeof(not_node_block_t));
	if (!basic)
	{
		not_error_no_memory();
		return NOT_PTR_ERROR;
	}

	basic->items = items;

	not_node_update(node, NODE_KIND_MODULE, basic);
	return node;
}
