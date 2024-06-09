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

static const char * const symbols[] = {
	[NODE_KIND_ID]			= "id",

	[NODE_KIND_NUMBER]		= "number",
	[NODE_KIND_CHAR]		= "char",
	[NODE_KIND_STRING]		= "string",

	[NODE_KIND_NULL]		= "null",

	[NODE_KIND_TUPLE]		= "tuple",
	[NODE_KIND_OBJECT]		= "object",

	[NODE_KIND_PSEUDONYM]	= "pseudonym",
	[NODE_KIND_THIS]		= "this",
	[NODE_KIND_SELF]		= "self",
	
	[NODE_KIND_KINT8]		= "int8",
	[NODE_KIND_KINT16]		= "int16",
	[NODE_KIND_KINT32]		= "int32",
	[NODE_KIND_KINT64]		= "int64",

	[NODE_KIND_KUINT8]		= "uint8",
	[NODE_KIND_KUINT16]		= "uint16",
	[NODE_KIND_KUINT32]		= "uint32",
	[NODE_KIND_KUINT64]		= "uint64",

	[NODE_KIND_KBIGINT]		= "bigint",

	[NODE_KIND_KFLOAT32]	= "float32",
	[NODE_KIND_KFLOAT64]	= "float64",
	[NODE_KIND_KBIGFLOAT]	= "bigfloat",

	[NODE_KIND_KCHAR]		= "char",
	[NODE_KIND_KSTRING]		= "string",

	[NODE_KIND_TYPEOF]		= "typeof",
	[NODE_KIND_SIZEOF]		= "sizeof",
	[NODE_KIND_PARENTHESIS]	= "parenthesis",
	
	[NODE_KIND_CALL]		= "call",
	[NODE_KIND_ARRAY]		= "array",
	[NODE_KIND_ATTRIBUTE]	= "attribute",
	
	[NODE_KIND_TILDE]		= "~",
	[NODE_KIND_NOT]			= "!",
	[NODE_KIND_NEG]			= "-",
	[NODE_KIND_POS]			= "+",
	
	[NODE_KIND_POW]			= "**",
	[NODE_KIND_EPI]			= "\\",

	[NODE_KIND_MUL]			= "*",
	[NODE_KIND_DIV]			= "/",
	[NODE_KIND_MOD]			= "%",
	
	[NODE_KIND_PLUS]		= "+",
	[NODE_KIND_MINUS]		= "-",
	
	[NODE_KIND_SHL]			= "<<",
	[NODE_KIND_SHR]			= ">>",
	
	[NODE_KIND_LT]			= "<",
	[NODE_KIND_LE]			= "<=",
	[NODE_KIND_GT]			= ">",
	[NODE_KIND_GE]			= ">=",
	
	[NODE_KIND_EQ]			= "==",
	[NODE_KIND_NEQ]			= "!=",
	
	[NODE_KIND_AND]			= "&",
	[NODE_KIND_XOR]			= "^",
	[NODE_KIND_OR]			= "|",
	[NODE_KIND_LAND]		= "&&",
	[NODE_KIND_LOR]			= "||",
	
	[NODE_KIND_CONDITIONAL] = "?",
	
	[NODE_KIND_ASSIGN]		= "=",
	[NODE_KIND_ADD_ASSIGN]	= "+=",
	[NODE_KIND_SUB_ASSIGN]	= "-=",
	[NODE_KIND_MUL_ASSIGN]	= "*=",
	[NODE_KIND_DIV_ASSIGN]	= "/=",
	[NODE_KIND_EPI_ASSIGN]	= "\\=",
	[NODE_KIND_MOD_ASSIGN]	= "%=",
	[NODE_KIND_AND_ASSIGN]	= "&=",
	[NODE_KIND_OR_ASSIGN]	= "|=",
	[NODE_KIND_SHL_ASSIGN]	= "<<=",
	[NODE_KIND_SHR_ASSIGN]	= ">>=",

	[NODE_KIND_IF]			= "if",
	[NODE_KIND_FOR]			= "for",
	[NODE_KIND_BREAK]		= "break",
	[NODE_KIND_CONTINUE]	= "continue",
	[NODE_KIND_CATCH]		= "catch",
	[NODE_KIND_TRY]			= "try",
	[NODE_KIND_RETURN]		= "return",
	[NODE_KIND_THROW]		= "throw",
	
	[NODE_KIND_VAR]			= "var",
	[NODE_KIND_LAMBDA]		= "lambda",
	[NODE_KIND_BODY]		= "body",
	[NODE_KIND_FUN]			= "fun",
	[NODE_KIND_CLASS]		= "class",
	[NODE_KIND_PAIR]		= "pair",
	[NODE_KIND_ENTITY]		= "entity",
	[NODE_KIND_SET]			= "set",
	[NODE_KIND_PROPERTY]	= "property",
	[NODE_KIND_NOTE]		= "note",
	[NODE_KIND_NOTES]		= "notes",
	[NODE_KIND_ARGUMENT]	= "argument",
	[NODE_KIND_ARGUMENTS]	= "arguments",
	[NODE_KIND_PARAMETER]	= "parameter",
	[NODE_KIND_PARAMETERS]	= "parameters",
	[NODE_KIND_FIELD]		= "field",
	[NODE_KIND_FIELDS]		= "fields",
	[NODE_KIND_GENERIC]		= "generic",
	[NODE_KIND_GENERICS]	= "generics",
	[NODE_KIND_HERITAGE]	= "heritage",
	[NODE_KIND_HERITAGES]	= "heritages",
	[NODE_KIND_PACKAGE]		= "package",
	[NODE_KIND_PACKAGES]	= "packages",
	[NODE_KIND_USING]		= "using",
	[NODE_KIND_MODULE]		= "module"
};

const char *
sy_node_kind_as_string(sy_node_t *node)
{
	return symbols[node->kind];
}

void
sy_node_destroy(sy_node_t *node)
{
  sy_memory_free (node);
}

sy_node_t *
sy_node_create(sy_node_t *parent, sy_position_t position)
{
	sy_node_t *node = (sy_node_t *)sy_memory_calloc(1, sizeof(sy_node_t));
	if(!node)
	{
		sy_error_no_memory();
		return NULL;
	}

	node->id = node_counter++;
	node->position = position;
	node->parent = parent;

	node->next = node->previous = NULL;

	return node;
}

static void
sy_node_Update(sy_node_t *node, int32_t kind, void *value)
{
	node->value = value;
	node->kind = kind;
}

void
sy_node_remove(sy_node_t *node)
{
	free(node);
}



sy_node_t *
sy_node_make_id(sy_node_t *node, char *value)
{
	sy_node_basic_t *basic = (sy_node_basic_t *)sy_memory_calloc(1, sizeof(sy_node_basic_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->value = value;

	sy_node_Update(node, NODE_KIND_ID, basic);
	return node;
}

sy_node_t *
sy_node_make_number(sy_node_t *node, char *value)
{
	sy_node_basic_t *basic = (sy_node_basic_t *)sy_memory_calloc(1, sizeof(sy_node_basic_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->value = value;

	sy_node_Update(node, NODE_KIND_NUMBER, basic);
	return node;
}

sy_node_t *
sy_node_make_char(sy_node_t *node, char *value)
{
	sy_node_basic_t *basic = (sy_node_basic_t *)sy_memory_calloc(1, sizeof(sy_node_basic_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->value = value;
	
	sy_node_Update(node, NODE_KIND_CHAR, basic);
	return node;
}

sy_node_t *
sy_node_make_string(sy_node_t *node, char *value)
{
	sy_node_basic_t *basic = (sy_node_basic_t *)sy_memory_calloc(1, sizeof(sy_node_basic_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->value = value;
	
	sy_node_Update(node, NODE_KIND_STRING, basic);
	return node;
}


sy_node_t *
sy_node_make_null(sy_node_t *node)
{
	sy_node_basic_t *basic = (sy_node_basic_t *)sy_memory_calloc(1, sizeof(sy_node_basic_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->value = NULL;
	
	sy_node_Update(node, NODE_KIND_NULL, basic);
	return node;
}

sy_node_t *
sy_node_make_kint8(sy_node_t *node)
{
	sy_node_basic_t *basic = (sy_node_basic_t *)sy_memory_calloc(1, sizeof(sy_node_basic_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->value = NULL;
	
	sy_node_Update(node, NODE_KIND_KINT8, basic);
	return node;
}

sy_node_t *
sy_node_make_kint16(sy_node_t *node)
{
	sy_node_basic_t *basic = (sy_node_basic_t *)sy_memory_calloc(1, sizeof(sy_node_basic_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->value = NULL;
	
	sy_node_Update(node, NODE_KIND_KINT16, basic);
	return node;
}

sy_node_t *
sy_node_make_kint32(sy_node_t *node)
{
	sy_node_basic_t *basic = (sy_node_basic_t *)sy_memory_calloc(1, sizeof(sy_node_basic_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->value = NULL;
	
	sy_node_Update(node, NODE_KIND_KINT32, basic);
	return node;
}

sy_node_t *
sy_node_make_kint64(sy_node_t *node)
{
	sy_node_basic_t *basic = (sy_node_basic_t *)sy_memory_calloc(1, sizeof(sy_node_basic_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->value = NULL;
	
	sy_node_Update(node, NODE_KIND_KINT64, basic);
	return node;
}


sy_node_t *
sy_node_make_kuint8(sy_node_t *node)
{
	sy_node_basic_t *basic = (sy_node_basic_t *)sy_memory_calloc(1, sizeof(sy_node_basic_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->value = NULL;
	
	sy_node_Update(node, NODE_KIND_KUINT8, basic);
	return node;
}

sy_node_t *
sy_node_make_kuint16(sy_node_t *node)
{
	sy_node_basic_t *basic = (sy_node_basic_t *)sy_memory_calloc(1, sizeof(sy_node_basic_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->value = NULL;
	
	sy_node_Update(node, NODE_KIND_KUINT16, basic);
	return node;
}

sy_node_t *
sy_node_make_kuint32(sy_node_t *node)
{
	sy_node_basic_t *basic = (sy_node_basic_t *)sy_memory_calloc(1, sizeof(sy_node_basic_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->value = NULL;
	
	sy_node_Update(node, NODE_KIND_KUINT32, basic);
	return node;
}

sy_node_t *
sy_node_make_kuint64(sy_node_t *node)
{
	sy_node_basic_t *basic = (sy_node_basic_t *)sy_memory_calloc(1, sizeof(sy_node_basic_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->value = NULL;
	
	sy_node_Update(node, NODE_KIND_KUINT64, basic);
	return node;
}

sy_node_t *
sy_node_make_kbigint(sy_node_t *node)
{
	sy_node_basic_t *basic = (sy_node_basic_t *)sy_memory_calloc(1, sizeof(sy_node_basic_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->value = NULL;
	
	sy_node_Update(node, NODE_KIND_KBIGINT, basic);
	return node;
}


sy_node_t *
sy_node_make_kfloat32(sy_node_t *node)
{
	sy_node_basic_t *basic = (sy_node_basic_t *)sy_memory_calloc(1, sizeof(sy_node_basic_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->value = NULL;
	
	sy_node_Update(node, NODE_KIND_KFLOAT32, basic);
	return node;
}

sy_node_t *
sy_node_make_kfloat64(sy_node_t *node)
{
	sy_node_basic_t *basic = (sy_node_basic_t *)sy_memory_calloc(1, sizeof(sy_node_basic_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->value = NULL;
	
	sy_node_Update(node, NODE_KIND_KFLOAT64, basic);
	return node;
}

sy_node_t *
sy_node_make_kbigfloat(sy_node_t *node)
{
	sy_node_basic_t *basic = (sy_node_basic_t *)sy_memory_calloc(1, sizeof(sy_node_basic_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->value = NULL;
	
	sy_node_Update(node, NODE_KIND_KBIGFLOAT, basic);
	return node;
}


sy_node_t *
sy_node_make_kchar(sy_node_t *node)
{
	sy_node_basic_t *basic = (sy_node_basic_t *)sy_memory_calloc(1, sizeof(sy_node_basic_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->value = NULL;
	
	sy_node_Update(node, NODE_KIND_KCHAR, basic);
	return node;
}

sy_node_t *
sy_node_make_kstring(sy_node_t *node)
{
	sy_node_basic_t *basic = (sy_node_basic_t *)sy_memory_calloc(1, sizeof(sy_node_basic_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->value = NULL;
	
	sy_node_Update(node, NODE_KIND_KSTRING, basic);
	return node;
}

sy_node_t *
sy_node_make_this(sy_node_t *node)
{
	sy_node_basic_t *basic = (sy_node_basic_t *)sy_memory_calloc(1, sizeof(sy_node_basic_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->value = NULL;
	
	sy_node_Update(node, NODE_KIND_THIS, basic);
	return node;
}

sy_node_t *
sy_node_make_self(sy_node_t *node)
{
	sy_node_basic_t *basic = (sy_node_basic_t *)sy_memory_calloc(1, sizeof(sy_node_basic_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->value = NULL;
	
	sy_node_Update(node, NODE_KIND_SELF, basic);
	return node;
}

sy_node_t *
sy_node_make_tuple(sy_node_t *node, sy_node_t *items)
{
	sy_node_block_t *basic = (sy_node_block_t *)sy_memory_calloc(1, sizeof(sy_node_block_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->items = items;
	
	sy_node_Update(node, NODE_KIND_TUPLE, basic);
	return node;
}

sy_node_t *
sy_node_make_object(sy_node_t *node, sy_node_t *items)
{
	sy_node_block_t *basic = (sy_node_block_t *)sy_memory_calloc(1, sizeof(sy_node_block_t));
	if(!basic){
		sy_error_no_memory();
		return NULL;
	}

	basic->items = items;
	
	sy_node_Update(node, NODE_KIND_OBJECT, basic);
	return node;
}

sy_node_t *
sy_node_make_pseudonym(sy_node_t *node, sy_node_t *base, sy_node_t *concepts)
{
	sy_node_carrier_t *basic = (sy_node_carrier_t *)sy_memory_calloc(1, sizeof(sy_node_carrier_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->base = base;
	basic->data = concepts;
	
	sy_node_Update(node, NODE_KIND_PSEUDONYM, basic);
	return node;
}


sy_node_t *
sy_node_make_typeof(sy_node_t *node, sy_node_t *right)
{
	sy_node_unary_t *basic = (sy_node_unary_t *)sy_memory_calloc(1, sizeof(sy_node_unary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->right = right;
	
	sy_node_Update(node, NODE_KIND_TYPEOF, basic);
	return node;
}

sy_node_t *
sy_node_make_sizeof(sy_node_t *node, sy_node_t *right)
{
	sy_node_unary_t *basic = (sy_node_unary_t *)sy_memory_calloc(1, sizeof(sy_node_unary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->right = right;
	
	sy_node_Update(node, NODE_KIND_SIZEOF, basic);
	return node;
}

sy_node_t *
sy_node_make_parenthesis(sy_node_t *node, sy_node_t *value)
{
	sy_node_unary_t *basic = (sy_node_unary_t *)sy_memory_calloc(1, sizeof(sy_node_unary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->right = value;
	
	sy_node_Update(node, NODE_KIND_PARENTHESIS, basic);
	return node;
}

sy_node_t *
sy_node_make_call(sy_node_t *node, sy_node_t *base, sy_node_t *arguments)
{
	sy_node_carrier_t *basic = (sy_node_carrier_t *)sy_memory_calloc(1, sizeof(sy_node_carrier_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->base = base;
	basic->data = arguments;
	
	sy_node_Update(node, NODE_KIND_CALL, basic);
	return node;
}

sy_node_t *
sy_node_make_array(sy_node_t *node, sy_node_t *base, sy_node_t *arguments)
{
	sy_node_carrier_t *basic = (sy_node_carrier_t *)sy_memory_calloc(1, sizeof(sy_node_carrier_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->base = base;
	basic->data = arguments;
	
	sy_node_Update(node, NODE_KIND_ARRAY, basic);
	return node;
}

sy_node_t *
sy_node_make_attribute(sy_node_t *node, sy_node_t *left, sy_node_t *right)
{
	sy_node_binary_t *basic = (sy_node_binary_t *)sy_memory_calloc(1, sizeof(sy_node_binary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->left = left;
	basic->right = right;
	
	sy_node_Update(node, NODE_KIND_ATTRIBUTE, basic);
	return node;
}

sy_node_t *
sy_node_make_tilde(sy_node_t *node, sy_node_t *right)
{
	sy_node_unary_t *basic = (sy_node_unary_t *)sy_memory_calloc(1, sizeof(sy_node_unary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->right = right;
	
	sy_node_Update(node, NODE_KIND_TILDE, basic);
	return node;
}

sy_node_t *
sy_node_make_not(sy_node_t *node, sy_node_t *right)
{
	sy_node_unary_t *basic = (sy_node_unary_t *)sy_memory_calloc(1, sizeof(sy_node_unary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->right = right;
	
	sy_node_Update(node, NODE_KIND_NOT, basic);
	return node;
}

sy_node_t *
sy_node_make_neg(sy_node_t *node, sy_node_t *right)
{
	sy_node_unary_t *basic = (sy_node_unary_t *)sy_memory_calloc(1, sizeof(sy_node_unary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->right = right;
	
	sy_node_Update(node, NODE_KIND_NEG, basic);
	return node;
}

sy_node_t *
sy_node_make_pos(sy_node_t *node, sy_node_t *right)
{
	sy_node_unary_t *basic = (sy_node_unary_t *)sy_memory_calloc(1, sizeof(sy_node_unary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->right = right;
	
	sy_node_Update(node, NODE_KIND_POS, basic);
	return node;
}

sy_node_t *
sy_node_make_pow(sy_node_t *node, sy_node_t *left, sy_node_t *right)
{
	sy_node_binary_t *basic = (sy_node_binary_t *)sy_memory_calloc(1, sizeof(sy_node_binary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->left = left;
	basic->right = right;
	
	sy_node_Update(node, NODE_KIND_POW, basic);
	return node;
}

sy_node_t *
sy_node_make_epi(sy_node_t *node, sy_node_t *left, sy_node_t *right)
{
	sy_node_binary_t *basic = (sy_node_binary_t *)sy_memory_calloc(1, sizeof(sy_node_binary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->left = left;
	basic->right = right;
	
	sy_node_Update(node, NODE_KIND_EPI, basic);
	return node;
}

sy_node_t *
sy_node_make_mul(sy_node_t *node, sy_node_t *left, sy_node_t *right)
{
	sy_node_binary_t *basic = (sy_node_binary_t *)sy_memory_calloc(1, sizeof(sy_node_binary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->left = left;
	basic->right = right;
	
	sy_node_Update(node, NODE_KIND_MUL, basic);
	return node;
}

sy_node_t *
sy_node_make_div(sy_node_t *node, sy_node_t *left, sy_node_t *right)
{
	sy_node_binary_t *basic = (sy_node_binary_t *)sy_memory_calloc(1, sizeof(sy_node_binary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->left = left;
	basic->right = right;
	
	sy_node_Update(node, NODE_KIND_DIV, basic);
	return node;
}

sy_node_t *
sy_node_make_mod(sy_node_t *node, sy_node_t *left, sy_node_t *right)
{
	sy_node_binary_t *basic = (sy_node_binary_t *)sy_memory_calloc(1, sizeof(sy_node_binary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->left = left;
	basic->right = right;
	
	sy_node_Update(node, NODE_KIND_MOD, basic);
	return node;
}

sy_node_t *
sy_node_make_plus(sy_node_t *node, sy_node_t *left, sy_node_t *right)
{
	sy_node_binary_t *basic = (sy_node_binary_t *)sy_memory_calloc(1, sizeof(sy_node_binary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->left = left;
	basic->right = right;
	
	sy_node_Update(node, NODE_KIND_PLUS, basic);
	return node;
}

sy_node_t *
sy_node_make_minus(sy_node_t *node, sy_node_t *left, sy_node_t *right)
{
	sy_node_binary_t *basic = (sy_node_binary_t *)sy_memory_calloc(1, sizeof(sy_node_binary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->left = left;
	basic->right = right;
	
	sy_node_Update(node, NODE_KIND_MINUS, basic);
	return node;
}

sy_node_t *
sy_node_make_shl(sy_node_t *node, sy_node_t *left, sy_node_t *right)
{
	sy_node_binary_t *basic = (sy_node_binary_t *)sy_memory_calloc(1, sizeof(sy_node_binary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->left = left;
	basic->right = right;
	
	sy_node_Update(node, NODE_KIND_SHL, basic);
	return node;
}

sy_node_t *
sy_node_make_shr(sy_node_t *node, sy_node_t *left, sy_node_t *right)
{
	sy_node_binary_t *basic = (sy_node_binary_t *)sy_memory_calloc(1, sizeof(sy_node_binary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->left = left;
	basic->right = right;
	
	sy_node_Update(node, NODE_KIND_SHR, basic);
	return node;
}

sy_node_t *
sy_node_make_lt(sy_node_t *node, sy_node_t *left, sy_node_t *right)
{
	sy_node_binary_t *basic = (sy_node_binary_t *)sy_memory_calloc(1, sizeof(sy_node_binary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->left = left;
	basic->right = right;
	
	sy_node_Update(node, NODE_KIND_LT, basic);
	return node;
}

sy_node_t *
sy_node_make_le(sy_node_t *node, sy_node_t *left, sy_node_t *right)
{
	sy_node_binary_t *basic = (sy_node_binary_t *)sy_memory_calloc(1, sizeof(sy_node_binary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->left = left;
	basic->right = right;
	
	sy_node_Update(node, NODE_KIND_LE, basic);
	return node;
}

sy_node_t *
sy_node_make_gt(sy_node_t *node, sy_node_t *left, sy_node_t *right)
{
	sy_node_binary_t *basic = (sy_node_binary_t *)sy_memory_calloc(1, sizeof(sy_node_binary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->left = left;
	basic->right = right;
	
	sy_node_Update(node, NODE_KIND_GT, basic);
	return node;
}

sy_node_t *
sy_node_make_ge(sy_node_t *node, sy_node_t *left, sy_node_t *right)
{
	sy_node_binary_t *basic = (sy_node_binary_t *)sy_memory_calloc(1, sizeof(sy_node_binary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->left = left;
	basic->right = right;
	
	sy_node_Update(node, NODE_KIND_GE, basic);
	return node;
}

sy_node_t *
sy_node_make_eq(sy_node_t *node, sy_node_t *left, sy_node_t *right)
{
	sy_node_binary_t *basic = (sy_node_binary_t *)sy_memory_calloc(1, sizeof(sy_node_binary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->left = left;
	basic->right = right;
	
	sy_node_Update(node, NODE_KIND_EQ, basic);
	return node;
}

sy_node_t *
sy_node_make_neq(sy_node_t *node, sy_node_t *left, sy_node_t *right)
{
	sy_node_binary_t *basic = (sy_node_binary_t *)sy_memory_calloc(1, sizeof(sy_node_binary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->left = left;
	basic->right = right;
	
	sy_node_Update(node, NODE_KIND_NEQ, basic);
	return node;
}

sy_node_t *
sy_node_make_and(sy_node_t *node, sy_node_t *left, sy_node_t *right)
{
	sy_node_binary_t *basic = (sy_node_binary_t *)sy_memory_calloc(1, sizeof(sy_node_binary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->left = left;
	basic->right = right;
	
	sy_node_Update(node, NODE_KIND_AND, basic);
	return node;
}

sy_node_t *
sy_node_make_xor(sy_node_t *node, sy_node_t *left, sy_node_t *right)
{
	sy_node_binary_t *basic = (sy_node_binary_t *)sy_memory_calloc(1, sizeof(sy_node_binary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->left = left;
	basic->right = right;
	
	sy_node_Update(node, NODE_KIND_XOR, basic);
	return node;
}

sy_node_t *
sy_node_make_or(sy_node_t *node, sy_node_t *left, sy_node_t *right)
{
	sy_node_binary_t *basic = (sy_node_binary_t *)sy_memory_calloc(1, sizeof(sy_node_binary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->left = left;
	basic->right = right;
	
	sy_node_Update(node, NODE_KIND_OR, basic);
	return node;
}

sy_node_t *
sy_node_make_land(sy_node_t *node, sy_node_t *left, sy_node_t *right)
{
	sy_node_binary_t *basic = (sy_node_binary_t *)sy_memory_calloc(1, sizeof(sy_node_binary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->left = left;
	basic->right = right;
	
	sy_node_Update(node, NODE_KIND_LAND, basic);
	return node;
}

sy_node_t *
sy_node_make_lor(sy_node_t *node, sy_node_t *left, sy_node_t *right)
{
	sy_node_binary_t *basic = (sy_node_binary_t *)sy_memory_calloc(1, sizeof(sy_node_binary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->left = left;
	basic->right = right;
	
	sy_node_Update(node, NODE_KIND_LOR, basic);
	return node;
}

sy_node_t *
sy_node_make_assign(sy_node_t *node, sy_node_t *left, sy_node_t *right)
{
	sy_node_binary_t *basic = (sy_node_binary_t *)sy_memory_calloc(1, sizeof(sy_node_binary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->left = left;
	basic->right = right;
	
	sy_node_Update(node, NODE_KIND_ASSIGN, basic);
	return node;
}

sy_node_t *
sy_node_make_add_assign(sy_node_t *node, sy_node_t *left, sy_node_t *right)
{
	sy_node_binary_t *basic = (sy_node_binary_t *)sy_memory_calloc(1, sizeof(sy_node_binary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->left = left;
	basic->right = right;
	
	sy_node_Update(node, NODE_KIND_ADD_ASSIGN, basic);
	return node;
}

sy_node_t *
sy_node_make_sub_assign(sy_node_t *node, sy_node_t *left, sy_node_t *right)
{
	sy_node_binary_t *basic = (sy_node_binary_t *)sy_memory_calloc(1, sizeof(sy_node_binary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->left = left;
	basic->right = right;
	
	sy_node_Update(node, NODE_KIND_SUB_ASSIGN, basic);
	return node;
}

sy_node_t *
sy_node_make_div_assign(sy_node_t *node, sy_node_t *left, sy_node_t *right)
{
	sy_node_binary_t *basic = (sy_node_binary_t *)sy_memory_calloc(1, sizeof(sy_node_binary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->left = left;
	basic->right = right;
	
	sy_node_Update(node, NODE_KIND_DIV_ASSIGN, basic);
	return node;
}

sy_node_t *
sy_node_make_epi_assign(sy_node_t *node, sy_node_t *left, sy_node_t *right)
{
	sy_node_binary_t *basic = (sy_node_binary_t *)sy_memory_calloc(1, sizeof(sy_node_binary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->left = left;
	basic->right = right;
	
	sy_node_Update(node, NODE_KIND_EPI_ASSIGN, basic);
	return node;
}

sy_node_t *
sy_node_make_mul_assign(sy_node_t *node, sy_node_t *left, sy_node_t *right)
{
	sy_node_binary_t *basic = (sy_node_binary_t *)sy_memory_calloc(1, sizeof(sy_node_binary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->left = left;
	basic->right = right;
	
	sy_node_Update(node, NODE_KIND_MUL_ASSIGN, basic);
	return node;
}

sy_node_t *
sy_node_make_mod_assign(sy_node_t *node, sy_node_t *left, sy_node_t *right)
{
	sy_node_binary_t *basic = (sy_node_binary_t *)sy_memory_calloc(1, sizeof(sy_node_binary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->left = left;
	basic->right = right;
	
	sy_node_Update(node, NODE_KIND_MOD_ASSIGN, basic);
	return node;
}

sy_node_t *
sy_node_make_pow_assign(sy_node_t *node, sy_node_t *left, sy_node_t *right)
{
	sy_node_binary_t *basic = (sy_node_binary_t *)sy_memory_calloc(1, sizeof(sy_node_binary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->left = left;
	basic->right = right;
	
	sy_node_Update(node, NODE_KIND_POW_ASSIGN, basic);
	return node;
}

sy_node_t *
sy_node_make_and_assign(sy_node_t *node, sy_node_t *left, sy_node_t *right)
{
	sy_node_binary_t *basic = (sy_node_binary_t *)sy_memory_calloc(1, sizeof(sy_node_binary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->left = left;
	basic->right = right;
	
	sy_node_Update(node, NODE_KIND_AND_ASSIGN, basic);
	return node;
}

sy_node_t *
sy_node_make_or_assign(sy_node_t *node, sy_node_t *left, sy_node_t *right)
{
	sy_node_binary_t *basic = (sy_node_binary_t *)sy_memory_calloc(1, sizeof(sy_node_binary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->left = left;
	basic->right = right;
	
	sy_node_Update(node, NODE_KIND_OR_ASSIGN, basic);
	return node;
}

sy_node_t *
sy_node_make_shl_assign(sy_node_t *node, sy_node_t *left, sy_node_t *right)
{
	sy_node_binary_t *basic = (sy_node_binary_t *)sy_memory_calloc(1, sizeof(sy_node_binary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->left = left;
	basic->right = right;
	
	sy_node_Update(node, NODE_KIND_SHL_ASSIGN, basic);
	return node;
}

sy_node_t *
sy_node_make_shr_assign(sy_node_t *node, sy_node_t *left, sy_node_t *right)
{
	sy_node_binary_t *basic = (sy_node_binary_t *)sy_memory_calloc(1, sizeof(sy_node_binary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->left = left;
	basic->right = right;
	
	sy_node_Update(node, NODE_KIND_SHR_ASSIGN, basic);
	return node;
}

sy_node_t *
sy_node_make_throw(sy_node_t *node, sy_node_t *value)
{
	sy_node_unary_t *basic = (sy_node_unary_t *)sy_memory_calloc(1, sizeof(sy_node_unary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->right = value;
	
	sy_node_Update(node, NODE_KIND_THROW, basic);
	return node;
}

sy_node_t *
sy_node_make_break(sy_node_t *node, sy_node_t *expression)
{
	sy_node_unary_t *basic = (sy_node_unary_t *)sy_memory_calloc(1, sizeof(sy_node_unary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->right = expression;

	sy_node_Update(node, NODE_KIND_BREAK, basic);
	return node;
}

sy_node_t *
sy_node_make_continue(sy_node_t *node, sy_node_t *expression)
{
	sy_node_unary_t *basic = (sy_node_unary_t *)sy_memory_calloc(1, sizeof(sy_node_unary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->right = expression;

	sy_node_Update(node, NODE_KIND_CONTINUE, basic);
	return node;
}

sy_node_t *
sy_node_make_return(sy_node_t *node, sy_node_t *expression)
{
	sy_node_unary_t *basic = (sy_node_unary_t *)sy_memory_calloc(1, sizeof(sy_node_unary_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->right = expression;
	
	sy_node_Update(node, NODE_KIND_RETURN, basic);
	return node;
}

sy_node_t *
sy_node_make_if(sy_node_t *node, sy_node_t *condition, sy_node_t *then_body, sy_node_t *else_body)
{
	sy_node_if_t *basic;
	if(!(basic = (sy_node_if_t *)sy_memory_calloc(1, sizeof(sy_node_if_t))))
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->condition = condition;
	basic->then_body = then_body;
	basic->else_body = else_body;
	
	sy_node_Update(node, NODE_KIND_IF, basic);
	return node;
}

sy_node_t *
sy_node_make_for(sy_node_t *node, sy_node_t *key, sy_node_t *initializer, sy_node_t *condition, sy_node_t *incrementor, sy_node_t *body)
{
	sy_node_for_t *basic = (sy_node_for_t *)sy_memory_calloc(1, sizeof(sy_node_for_t));
	if(!basic){
		sy_error_no_memory();
		return NULL;
	}

	basic->key = key;
	basic->initializer = initializer;
	basic->condition = condition;
	basic->incrementor = incrementor;
	basic->body = body;
	
	sy_node_Update(node, NODE_KIND_FOR, basic);
	return node;
}

sy_node_t *
sy_node_make_catch(sy_node_t *node, sy_node_t *parameters, sy_node_t *body, sy_node_t *next)
{
	sy_node_catch_t *basic = (sy_node_catch_t *)sy_memory_calloc(1, sizeof(sy_node_catch_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->parameters = parameters;
	basic->body = body;
	basic->next = next;
	
	sy_node_Update(node, NODE_KIND_CATCH, basic);
	return node;
}

sy_node_t *
sy_node_make_try(sy_node_t *node, sy_node_t *body, sy_node_t *catchs)
{
	sy_node_try_t *basic;
	if(!(basic = (sy_node_try_t *)sy_memory_calloc(1, sizeof(sy_node_try_t))))
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->body = body;
	basic->catchs = catchs;
	
	sy_node_Update(node, NODE_KIND_TRY, basic);
	return node;
}

sy_node_t *
sy_node_make_var(sy_node_t *node, uint64_t flag, sy_node_t *key, sy_node_t *type, sy_node_t *value){
	sy_node_var_t *basic = (sy_node_var_t *)sy_memory_calloc(1, sizeof(sy_node_var_t));

	if(!basic){
		sy_error_no_memory();
		return NULL;
	}

	basic->flag = flag;
	basic->key = key;
	basic->type = type;
	basic->value = value;
	
	sy_node_Update(node, NODE_KIND_VAR, basic);
	return node;
}

sy_node_t *
sy_node_make_argument(sy_node_t *node, sy_node_t *key, sy_node_t *value)
{
	sy_node_argument_t *basic = (sy_node_argument_t *)sy_memory_calloc(1, sizeof(sy_node_argument_t));
	if(!basic){
		sy_error_no_memory();
		return NULL;
	}

	basic->key = key;
	basic->value = value;
	
	sy_node_Update(node, NODE_KIND_ARGUMENT, basic);
	return node;
}

sy_node_t *
sy_node_make_arguments(sy_node_t *node, sy_node_t *items)
{
	sy_node_block_t *basic = (sy_node_block_t *)sy_memory_calloc(1, sizeof(sy_node_block_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->items = items;
	
	sy_node_Update(node, NODE_KIND_ARGUMENTS, basic);
	return node;
}

sy_node_t *
sy_node_make_parameter(sy_node_t *node, uint64_t flag, sy_node_t *key, sy_node_t *type, sy_node_t *value){
	sy_node_parameter_t *basic = (sy_node_parameter_t *)sy_memory_calloc(1, sizeof(sy_node_parameter_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->flag = flag;
	basic->key = key;
	basic->type = type;
	basic->value = value;
	
	sy_node_Update(node, NODE_KIND_PARAMETER, basic);
	return node;
}

sy_node_t *
sy_node_make_parameters(sy_node_t *node, sy_node_t *items)
{
	sy_node_block_t *basic = (sy_node_block_t *)sy_memory_calloc(1, sizeof(sy_node_block_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->items = items;
	
	sy_node_Update(node, NODE_KIND_PARAMETERS, basic);
	return node;
}

sy_node_t *
sy_node_make_field(sy_node_t *node, sy_node_t *key, sy_node_t *value)
{
	sy_node_field_t *basic = (sy_node_field_t *)sy_memory_calloc(1, sizeof(sy_node_field_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->key = key;
	basic->value = value;
	
	sy_node_Update(node, NODE_KIND_FIELD, basic);
	return node;
}

sy_node_t *
sy_node_make_fields(sy_node_t *node, sy_node_t *items)
{
	sy_node_block_t *basic = (sy_node_block_t *)sy_memory_calloc(1, sizeof(sy_node_block_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->items = items;
	
	sy_node_Update(node, NODE_KIND_PARAMETERS, basic);
	return node;
}

sy_node_t *
sy_node_make_generic(sy_node_t *node, sy_node_t *key, sy_node_t *type, sy_node_t *value)
{
	sy_node_generic_t *basic = (sy_node_generic_t *)sy_memory_calloc(1, sizeof(sy_node_generic_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->key = key;
	basic->type = type;
	basic->value = value;
	
	sy_node_Update(node, NODE_KIND_GENERIC, basic);
	return node;
}

sy_node_t *
sy_node_make_generics(sy_node_t *node, sy_node_t *items)
{
	sy_node_block_t *basic = (sy_node_block_t *)sy_memory_calloc(1, sizeof(sy_node_block_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->items = items;
	
	sy_node_Update(node, NODE_KIND_PARAMETERS, basic);
	return node;
}

sy_node_t *
sy_node_make_func(sy_node_t *node, sy_node_t *notes, uint64_t flag, sy_node_t *key, sy_node_t *generics, sy_node_t *parameters, sy_node_t *result, sy_node_t *body)
{
	sy_node_fun_t *basic = (sy_node_fun_t *)sy_memory_calloc(1, sizeof(sy_node_fun_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->flag = flag;
	basic->notes = notes;
	basic->key = key;
	basic->generics = generics;
	basic->parameters = parameters;
	basic->result = result;
	basic->body = body;
	
	sy_node_Update(node, NODE_KIND_FUN, basic);
	return node;
}

sy_node_t *
sy_node_make_lambda(sy_node_t *node, sy_node_t *generics, sy_node_t *parameters, sy_node_t *body, sy_node_t *result)
{
	sy_node_lambda_t *basic = (sy_node_lambda_t *)sy_memory_calloc(1, sizeof(sy_node_lambda_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->generics = generics;
	basic->parameters = parameters;
	basic->body = body;
	basic->result= result;
	
	sy_node_Update(node, NODE_KIND_LAMBDA, basic);
	return node;
}

sy_node_t *
sy_node_make_property(sy_node_t *node, sy_node_t *notes, uint64_t flag, sy_node_t *key, sy_node_t *type, sy_node_t *value)
{
	sy_node_property_t *basic = (sy_node_property_t *)sy_memory_calloc(1, sizeof(sy_node_property_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->flag = flag;
	basic->notes = notes;
	basic->key = key;
	basic->type = type;
	basic->value = value;
	
	sy_node_Update(node, NODE_KIND_PROPERTY, basic);
	return node;
}

sy_node_t *
sy_node_make_entity(sy_node_t *node, uint64_t flag, sy_node_t *key, sy_node_t *type, sy_node_t *value)
{
	sy_node_entity_t *basic = (sy_node_entity_t *)sy_memory_calloc(1, sizeof(sy_node_entity_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->flag = flag;
	basic->key = key;
	basic->type = type;
	basic->value = value;
	
	sy_node_Update(node, NODE_KIND_ENTITY, basic);
	return node;
}

sy_node_t *
sy_node_make_set(sy_node_t *node, sy_node_t *items)
{
	sy_node_block_t *basic = (sy_node_block_t *)sy_memory_calloc(1, sizeof(sy_node_block_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->items = items;
	
	sy_node_Update(node, NODE_KIND_SET, basic);
	return node;
}

sy_node_t *
sy_node_make_pair(sy_node_t *node, sy_node_t *key, sy_node_t *value)
{
	sy_node_pair_t *basic = (sy_node_pair_t *)sy_memory_calloc(1, sizeof(sy_node_pair_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->key = key;
	basic->value = value;
	
	sy_node_Update(node, NODE_KIND_PAIR, basic);
	return node;
}

sy_node_t *
sy_node_make_heritage(sy_node_t *node, sy_node_t *key, sy_node_t *type)
{
	sy_node_heritage_t *basic = (sy_node_heritage_t *)sy_memory_calloc(1, sizeof(sy_node_heritage_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->key = key;
	basic->type = type;
	
	sy_node_Update(node, NODE_KIND_HERITAGE, basic);
	return node;
}

sy_node_t *
sy_node_make_heritages(sy_node_t *node, sy_node_t *items)
{
	sy_node_block_t *basic = (sy_node_block_t *)sy_memory_calloc(1, sizeof(sy_node_block_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->items = items;
	
	sy_node_Update(node, NODE_KIND_HERITAGES, basic);
	return node;
}

sy_node_t *
sy_node_make_class(sy_node_t *node, sy_node_t *notes, uint64_t flag, sy_node_t *key, sy_node_t *generics, sy_node_t *heritages, sy_node_t *block)
{
	sy_node_class_t *basic = (sy_node_class_t *)sy_memory_calloc(1, sizeof(sy_node_class_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->flag = flag;
	basic->notes = notes;
	basic->key = key;
	basic->generics = generics;
	basic->heritages = heritages;
	basic->block = block;
	
	sy_node_Update(node, NODE_KIND_CLASS, basic);
	return node;
}

sy_node_t *
sy_node_make_note(sy_node_t *node, sy_node_t *key, sy_node_t *arguments)
{
	sy_node_carrier_t *basic = (sy_node_carrier_t *)sy_memory_calloc(1, sizeof(sy_node_carrier_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}
	
	basic->base = key;
	basic->data = arguments;
	
	sy_node_Update(node, NODE_KIND_NOTE, basic);
	return node;
}

sy_node_t *
sy_node_make_notes(sy_node_t *node, sy_node_t *items)
{
	sy_node_block_t *basic = (sy_node_block_t *)sy_memory_calloc(1, sizeof(sy_node_block_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->items = items;
	
	sy_node_Update(node, NODE_KIND_NOTES, basic);
	return node;
}

sy_node_t *
sy_node_make_package(sy_node_t *node, sy_node_t *key, sy_node_t *value)
{
	sy_node_package_t *basic = (sy_node_package_t *)sy_memory_calloc(1, sizeof(sy_node_package_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->key = key;
	basic->value = value;
	
	sy_node_Update(node, NODE_KIND_PACKAGE, basic);
	return node;
}

sy_node_t *
sy_node_make_packages(sy_node_t *node, sy_node_t *items)
{
	sy_node_block_t *basic = (sy_node_block_t *)sy_memory_calloc(1, sizeof(sy_node_block_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->items = items;
	
	sy_node_Update(node, NODE_KIND_PACKAGES, basic);
	return node;
}

sy_node_t *
sy_node_make_using(sy_node_t *node, sy_node_t *path, sy_node_t *packages)
{
	sy_node_using_t *basic = (sy_node_using_t *)sy_memory_calloc(1, sizeof(sy_node_using_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->path = path;
	basic->packages = packages;
	
	sy_node_Update(node, NODE_KIND_USING, basic);
	return node;
}

sy_node_t *
sy_node_make_body(sy_node_t *node, sy_node_t *declaration)
{
	sy_node_body_t *basic = (sy_node_body_t *)sy_memory_calloc(1, sizeof(sy_node_body_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->declaration = declaration;
	
	sy_node_Update(node, NODE_KIND_BODY, basic);
	return node;
}

sy_node_t *
sy_node_make_module(sy_node_t *node, sy_node_t *items)
{
	sy_node_block_t *basic = (sy_node_block_t *)sy_memory_calloc(1, sizeof(sy_node_block_t));
	if(!basic)
	{
		sy_error_no_memory();
		return NULL;
	}

	basic->items = items;
	
	sy_node_Update(node, NODE_KIND_MODULE, basic);
	return node;
}
