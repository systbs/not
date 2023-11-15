#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
#include "../utils/dirent.h"
#elif defined(__arm__) || defined(__aarch64__)
#include "../utils/arm/dirent.h"
#else
#include <dirent.h>
#endif

#include "../types/types.h"
#include "../container/list.h"
#include "../token/position.h"
#include "../token/token.h"
#include "../program.h"
#include "../scanner/file.h"
#include "../scanner/scanner.h"
#include "../ast/node.h"
#include "../utils/utils.h"
#include "../utils/path.h"
#include "parser.h"
#include "error.h"
#include "syntax.h"
#include "symbol.h"

static error_t *
syntax_error(syntax_t *syntax, position_t position, const char *format, ...)
{
	char *message;
	message = malloc(1024);
	if (!message)
	{
		return NULL;
	}

	va_list arg;
	if (format)
	{
		va_start(arg, format);
		vsprintf(message, format, arg);
		va_end(arg);
	}

	error_t *error;
	error = error_create(position, message);
	if (!error)
	{
		return NULL;
	}

	if (list_rpush(syntax->errors, (list_value_t)error))
	{
		return NULL;
	}

	return error;
}

static int32_t
syntax_module(syntax_t *syntax, node_t *node);

static int32_t
syntax_fetch_import_path(syntax_t *syntax, node_t *node, char *base_path)
{
	node_basic_t *node_basic;
	node_basic = (node_basic_t *)node->value;

	char *path = node_basic->value;

	char directory_path[_MAX_DIR];
	char resolved_path[_MAX_DIR + _MAX_FNAME + _MAX_EXT];

	int32_t is_root = 0;
	int32_t is_absolute = 0;
	if ((is_root = path_is_root(path)))
	{
		path = path + 2;
		char *temp = getenv("QALAM-PATH");
		if (!temp)
		{
			syntax_error(syntax, node->position, "root path is null, add 'QALAM-PATH' to environment");
			return 0;
		}
		strcpy(directory_path, temp);
		path_join(directory_path, path, resolved_path, sizeof(resolved_path));
	}
	else if ((is_absolute = path_is_absolute(path)))
	{
		strcpy(resolved_path, path);
	}
	else
	{
		path_get_directory_path(base_path, directory_path, sizeof(directory_path));
		path_join(directory_path, path, resolved_path, sizeof(resolved_path));
	}

	if (path_is_directory(resolved_path))
	{
		DIR *dr;
		struct dirent *dirent;
		dr = opendir(resolved_path);
		if (dr)
		{
			while ((dirent = readdir(dr)) != NULL)
			{
				char *ext;
				ext = strrchr(dirent->d_name, '.');
				if (!!ext && strcmp(ext, ".q") == 0)
				{
					char resolved_file[_MAX_DIR + _MAX_FNAME + _MAX_EXT];
					path_join(resolved_path, dirent->d_name, resolved_file, sizeof(resolved_file));

					int32_t exist_module = 0;
					ilist_t *a;
					for (a = syntax->modules->begin; a != syntax->modules->end; a = a->next)
					{
						node_t *temp;
						temp = (node_t *)a->value;

						node_module_t *node_module;
						node_module = (node_module_t *)temp->value;
						if (strcmp(node_module->path, resolved_file) == 0)
						{
							exist_module = 1;
							break;
						}
					}
					if (exist_module)
					{
						continue;
					}

					parser_t *parser;
					parser = parser_create(syntax->program, resolved_file, syntax->errors);
					if (!parser)
					{
						return 0;
					}

					node_t *module;
					module = parser_module(parser);
					if (!module)
					{
						return 0;
					}

					if (!syntax_module(syntax, module))
					{
						return 0;
					}
				}
			}
			closedir(dr);
		}
		else
		{
			syntax_error(syntax, node->position, "could not open dir '%s'", resolved_path);
			return 0;
		}
	}
	else
	{
		if (!path_exist(resolved_path))
		{
			if (is_absolute)
			{
				syntax_error(syntax, node->position, "module '%s' is not found", resolved_path);
				return 0;
			}
			syntax_error(syntax, node->position, "module '%s' is not in '%s'", resolved_path, directory_path);
			return 0;
		}

		int32_t exist_module = 0;
		ilist_t *a;
		for (a = syntax->modules->begin; a != syntax->modules->end; a = a->next)
		{
			node_t *temp;
			temp = (node_t *)a->value;

			node_module_t *node_module;
			node_module = (node_module_t *)temp->value;
			if (strcmp(node_module->path, resolved_path) == 0)
			{
				exist_module = 1;
				break;
			}
		}
		if (exist_module)
		{
			return 1;
		}

		parser_t *parser;
		parser = parser_create(syntax->program, resolved_path, syntax->errors);
		if (!parser)
		{
			return 0;
		}

		node_t *module;
		module = parser_module(parser);
		if (!module)
		{
			return 0;
		}

		if (!syntax_module(syntax, module))
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
syntax_fetch_import(syntax_t *syntax, node_t *node, char *base_path)
{
	node_import_t *node_import;
	node_import = (node_import_t *)node->value;

	int32_t result;
	result = syntax_fetch_import_path(syntax, node_import->path, base_path);
	if (!result)
	{
		return 0;
	}

	return 1;
}

static int32_t
syntax_apply_import(syntax_t *syntax, node_t *node)
{
	node_module_t *node_module;
	node_module = (node_module_t *)node->value;

	int32_t result;
	ilist_t *a;
	for (a = node_module->members->begin; a != node_module->members->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;
		if (temp->kind == NODE_KIND_IMPORT)
		{
			result = syntax_fetch_import(syntax, temp, node_module->path);
			if (!result)
			{
				return 0;
			}
		}
	}

	return 1;
}

static int32_t
syntax_expression(syntax_t *syntax, node_t *parent, node_t *node);

static int32_t
syntax_func(syntax_t *syntax, node_t *parent, node_t *node);

static int32_t
syntax_parameter(syntax_t *syntax, node_t *parent, node_t *node);

static int32_t
syntax_type_parameter(syntax_t *syntax, node_t *parent, node_t *node);

static int32_t
syntax_postfix(syntax_t *syntax, node_t *parent, node_t *node);

static int32_t
syntax_block(syntax_t *syntax, node_t *parent, node_t *node);

static int32_t
syntax_const(syntax_t *syntax, node_t *parent, node_t *node);

static int32_t
syntax_var(syntax_t *syntax, node_t *parent, node_t *node);

static int32_t
syntax_type(syntax_t *syntax, node_t *parent, node_t *node);

static int32_t
syntax_prefix(syntax_t *syntax, node_t *parent, node_t *node);

static int32_t
syntax_export(syntax_t *syntax, node_t *parent, node_t *node);

static int32_t
syntax_check_is_name(syntax_t *syntax, node_t *node);


static int32_t
syntax_id(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;
	return 1;
}

static int32_t
syntax_number(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;
	return 1;
}

static int32_t
syntax_string(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;
	return 1;
}

static int32_t
syntax_char(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;
	return 1;
}

static int32_t
syntax_null(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;
	return 1;
}

static int32_t
syntax_true(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;
	return 1;
}

static int32_t
syntax_false(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;
	return 1;
}

static int32_t
syntax_array(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_array_t *node_array;
	node_array = (node_array_t *)node->value;

	int32_t result;
	ilist_t *a;
	for (a = node_array->list->begin; a != node_array->list->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		result = syntax_expression(syntax, node, temp);
		if (!result)
		{
			return 0;
		}
	}
	return 1;
}

static int32_t
syntax_parenthesis(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_unary_t *node_unary;
	node_unary = (node_unary_t *)node->value;

	int32_t result;
	result = syntax_expression(syntax, node, node_unary->right);
	if (!result)
	{
		return 0;
	}

	return 1;
}

static int32_t
syntax_object_property(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_object_property_t *node_object_property;
	node_object_property = (node_object_property_t *)node->value;

	int32_t result;
	result = syntax_expression(syntax, node, node_object_property->name);
	if (!result)
	{
		return 0;
	}

	if (node_object_property->value)
	{
		result = syntax_expression(syntax, node, node_object_property->value);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
syntax_object(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_object_t *node_object;
	node_object = (node_object_t *)node->value;

	int32_t result;
	ilist_t *a;
	for (a = node_object->list->begin; a != node_object->list->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		result = syntax_object_property(syntax, node, temp);
		if (!result)
		{
			return 0;
		}
	}
	return 1;
}

static int32_t
syntax_primary(syntax_t *syntax, node_t *parent, node_t *node)
{
	int32_t result;
	switch (node->kind)
	{
	case NODE_KIND_ID:
		result = syntax_id(syntax, parent, node);
		break;

	case NODE_KIND_NUMBER:
		result = syntax_number(syntax, parent, node);
		break;

	case NODE_KIND_STRING:
		result = syntax_string(syntax, parent, node);
		break;

	case NODE_KIND_CHAR:
		result = syntax_char(syntax, parent, node);
		break;

	case NODE_KIND_NULL:
		result = syntax_null(syntax, parent, node);
		break;

	case NODE_KIND_TRUE:
		result = syntax_true(syntax, parent, node);
		break;

	case NODE_KIND_FALSE:
		result = syntax_false(syntax, parent, node);
		break;

	case NODE_KIND_ARRAY:
		result = syntax_array(syntax, parent, node);
		break;

	case NODE_KIND_OBJECT:
		result = syntax_object(syntax, parent, node);
		break;

	case NODE_KIND_FUNC:
		result = syntax_func(syntax, parent, node);
		break;

	case NODE_KIND_PARENTHESIS:
		result = syntax_parenthesis(syntax, parent, node);
		break;

	default:
		syntax_error(syntax, node->position, "unknown primary node %ld", node->kind);
		return 0;
	}

	if (!result)
	{
		return 0;
	}

	return 1;
}

static int32_t
syntax_check_cannot_followed_by_composite(syntax_t *syntax, node_t *node)
{
	node_composite_t *node_composite;
	node_composite = (node_composite_t *)node->value;

	if (node_composite->base->kind == NODE_KIND_GET_SLICE)
	{
		syntax_error(syntax, node->position,
									"A slice has no signatures for which the type argument list is applicable");
		return 0;
	}
	return 1;
}

static int32_t
syntax_composite(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;

	syntax_check_cannot_followed_by_composite(syntax, node);

	node_composite_t *node_composite;
	node_composite = (node_composite_t *)node->value;

	int32_t result;
	result = syntax_postfix(syntax, node, node_composite->base);
	if (!result)
	{
		return 0;
	}

	if (node_composite->type_arguments)
	{
		ilist_t *a;
		for (a = node_composite->type_arguments->begin; a != node_composite->type_arguments->end; a = a->next)
		{
			node_t *temp;
			temp = (node_t *)a->value;

			result = syntax_expression(syntax, node, temp);
			if (!result)
			{
				return 0;
			}
		}
	}

	return 1;
}

static int32_t
syntax_check_cannot_followed_by_call(syntax_t *syntax, node_t *node)
{
	node_call_t *node_call;
	node_call = (node_call_t *)node->value;

	if (node_call->callable->kind == NODE_KIND_GET_SLICE)
	{
		syntax_error(syntax, node->position, "A slice cannot be called");
		return 0;
	}
	return 1;
}

static int32_t
syntax_call(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;

	syntax_check_cannot_followed_by_call(syntax, node);

	node_call_t *node_call;
	node_call = (node_call_t *)node->value;

	int32_t result;
	result = syntax_postfix(syntax, node, node_call->callable);
	if (!result)
	{
		return 0;
	}

	if (node_call->arguments)
	{
		ilist_t *a;
		for (a = node_call->arguments->begin; a != node_call->arguments->end; a = a->next)
		{
			node_t *temp;
			temp = (node_t *)a->value;

			result = syntax_expression(syntax, node, temp);
			if (!result)
			{
				return 0;
			}
		}
	}

	return 1;
}

static int32_t
syntax_check_cannot_followed_by_get_slice(syntax_t *syntax, node_t *node)
{
	node_get_slice_t *node_get_slice;
	node_get_slice = (node_get_slice_t *)node->value;

	if (node_get_slice->name->kind == NODE_KIND_COMPOSITE)
	{
		syntax_error(syntax, node->position, "An instantiation expression cannot be followed by a property access");
		return 0;
	}
	return 1;
}

static int32_t
syntax_get_slice(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;

	syntax_check_cannot_followed_by_get_slice(syntax, node);

	node_get_slice_t *node_get_slice;
	node_get_slice = (node_get_slice_t *)node->value;

	int32_t result;
	result = syntax_postfix(syntax, node, node_get_slice->name);
	if (!result)
	{
		return 0;
	}

	if (node_get_slice->start)
	{
		result = syntax_expression(syntax, node, node_get_slice->start);
		if (!result)
		{
			return 0;
		}
	}

	if (node_get_slice->step)
	{
		result = syntax_expression(syntax, node, node_get_slice->step);
		if (!result)
		{
			return 0;
		}
	}

	if (node_get_slice->stop)
	{
		result = syntax_expression(syntax, node, node_get_slice->stop);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
syntax_check_cannot_followed_by_get_item(syntax_t *syntax, node_t *node)
{
	node_get_item_t *node_get_item;
	node_get_item = (node_get_item_t *)node->value;

	if (node_get_item->name->kind == NODE_KIND_COMPOSITE)
	{
		syntax_error(syntax, node->position, "An instantiation expression cannot be followed by a property access");
		return 0;
	}
	return 1;
}

static int32_t
syntax_get_item(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;

	syntax_check_cannot_followed_by_get_item(syntax, node);

	node_get_item_t *node_get_item;
	node_get_item = (node_get_item_t *)node->value;

	int32_t result;
	result = syntax_postfix(syntax, node, node_get_item->name);
	if (!result)
	{
		return 0;
	}

	if (node_get_item->index)
	{
		result = syntax_expression(syntax, node, node_get_item->index);
		if (!result)
		{
			return 0;
		}
	}
	return 1;
}

static int32_t
syntax_check_cannot_followed_by_get_attr(syntax_t *syntax, node_t *node)
{
	node_get_attr_t *node_get_attr;
	node_get_attr = (node_get_attr_t *)node->value;

	if (node_get_attr->left->kind == NODE_KIND_GET_SLICE)
	{
		syntax_error(syntax, node->position, "A slice cannot be followed by a property access");
		return 0;
	}
	return 1;
}

static int32_t
syntax_get_attr(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;

	syntax_check_cannot_followed_by_get_attr(syntax, node);

	node_get_attr_t *node_get_attr;
	node_get_attr = (node_get_attr_t *)node->value;

	int32_t result;
	result = syntax_postfix(syntax, node, node_get_attr->left);
	if (!result)
	{
		return 0;
	}

	result = syntax_id(syntax, node, node_get_attr->right);
	if (!result)
	{
		return 0;
	}
	return 1;
}

static int32_t
syntax_postfix(syntax_t *syntax, node_t *parent, node_t *node)
{
	int32_t result;
	switch (node->kind)
	{
	case NODE_KIND_COMPOSITE:
		result = syntax_composite(syntax, parent, node);
		if (!result)
		{
			return 0;
		}
		break;

	case NODE_KIND_CALL:
		result = syntax_call(syntax, parent, node);
		if (!result)
		{
			return 0;
		}
		break;

	case NODE_KIND_GET_SLICE:
		result = syntax_get_slice(syntax, parent, node);
		if (!result)
		{
			return 0;
		}
		break;

	case NODE_KIND_GET_ITEM:
		result = syntax_get_item(syntax, parent, node);
		if (!result)
		{
			return 0;
		}
		break;

	case NODE_KIND_GET_ATTR:
		result = syntax_get_attr(syntax, parent, node);
		if (!result)
		{
			return 0;
		}
		break;

	default:
		result = syntax_primary(syntax, parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}
	return 1;
}

static int32_t
syntax_prefix(syntax_t *syntax, node_t *parent, node_t *node)
{
	int32_t result;
	node_unary_t *node_unary;
	switch (node->kind)
	{
	case NODE_KIND_TILDE:
	case NODE_KIND_NOT:
	case NODE_KIND_NEG:
	case NODE_KIND_POS:
	case NODE_KIND_GET_VALUE:
	case NODE_KIND_GET_ADDRESS:
	case NODE_KIND_AWAIT:
	case NODE_KIND_SIZEOF:
	case NODE_KIND_TYPEOF:
	case NODE_KIND_ELLIPSIS:
		node->parent = parent;
		node_unary = (node_unary_t *)node->value;
		if (node_unary->right->kind == NODE_KIND_TILDE)
		{
			result = syntax_prefix(syntax, node, node_unary->right);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_unary->right->kind == NODE_KIND_NOT)
		{
			result = syntax_prefix(syntax, node, node_unary->right);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_unary->right->kind == NODE_KIND_POS)
		{
			result = syntax_prefix(syntax, node, node_unary->right);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_unary->right->kind == NODE_KIND_NEG)
		{
			result = syntax_prefix(syntax, node, node_unary->right);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_unary->right->kind == NODE_KIND_GET_VALUE)
		{
			result = syntax_prefix(syntax, node, node_unary->right);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_unary->right->kind == NODE_KIND_GET_ADDRESS)
		{
			result = syntax_prefix(syntax, node, node_unary->right);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_unary->right->kind == NODE_KIND_AWAIT)
		{
			result = syntax_prefix(syntax, node, node_unary->right);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_unary->right->kind == NODE_KIND_SIZEOF)
		{
			result = syntax_prefix(syntax, node, node_unary->right);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_unary->right->kind == NODE_KIND_TYPEOF)
		{
			result = syntax_prefix(syntax, node, node_unary->right);
			if (!result)
			{
				return 0;
			}
		}
		else
		{
			result = syntax_postfix(syntax, node, node_unary->right);
			if (!result)
			{
				return 0;
			}
		}
		break;

	default:
		result = syntax_postfix(syntax, parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}

	return 1;
}

static int32_t
syntax_multiplicative(syntax_t *syntax, node_t *parent, node_t *node)
{
	int32_t result;
	node_binary_t *node_binary;
	switch (node->kind)
	{
	case NODE_KIND_MUL:
	case NODE_KIND_DIV:
	case NODE_KIND_MOD:
		node->parent = parent;
		node_binary = (node_binary_t *)node->value;
		if (node_binary->left->kind == NODE_KIND_MUL)
		{
			result = syntax_multiplicative(syntax, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_binary->left->kind == NODE_KIND_DIV)
		{
			result = syntax_multiplicative(syntax, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_binary->left->kind == NODE_KIND_MOD)
		{
			result = syntax_multiplicative(syntax, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else
		{
			result = syntax_prefix(syntax, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}

		result = syntax_prefix(syntax, node, node_binary->right);
		if (!result)
		{
			return 0;
		}
		break;

	default:
		result = syntax_prefix(syntax, parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}

	return 1;
}

static int32_t
syntax_addative(syntax_t *syntax, node_t *parent, node_t *node)
{
	int32_t result;
	node_binary_t *node_binary;
	switch (node->kind)
	{
	case NODE_KIND_PLUS:
	case NODE_KIND_MINUS:
		node->parent = parent;
		node_binary = (node_binary_t *)node->value;
		if (node_binary->left->kind == NODE_KIND_PLUS)
		{
			result = syntax_addative(syntax, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_binary->left->kind == NODE_KIND_MINUS)
		{
			result = syntax_addative(syntax, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else
		{
			result = syntax_multiplicative(syntax, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}

		result = syntax_multiplicative(syntax, node, node_binary->right);
		if (!result)
		{
			return 0;
		}
		break;

	default:
		result = syntax_multiplicative(syntax, parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}

	return 1;
}

static int32_t
syntax_shifting(syntax_t *syntax, node_t *parent, node_t *node)
{
	int32_t result;
	node_binary_t *node_binary;
	switch (node->kind)
	{
	case NODE_KIND_SHL:
	case NODE_KIND_SHR:
		node->parent = parent;
		node_binary = (node_binary_t *)node->value;
		if (node_binary->left->kind == NODE_KIND_SHL)
		{
			result = syntax_shifting(syntax, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_binary->left->kind == NODE_KIND_SHR)
		{
			result = syntax_shifting(syntax, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else
		{
			result = syntax_addative(syntax, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}

		result = syntax_addative(syntax, node, node_binary->right);
		if (!result)
		{
			return 0;
		}
		break;

	default:
		result = syntax_addative(syntax, parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}

	return 1;
}

static int32_t
syntax_relational(syntax_t *syntax, node_t *parent, node_t *node)
{
	int32_t result;
	node_binary_t *node_binary;
	switch (node->kind)
	{
	case NODE_KIND_LT:
	case NODE_KIND_GT:
	case NODE_KIND_LE:
	case NODE_KIND_GE:
		node->parent = parent;
		node_binary = (node_binary_t *)node->value;
		if (node_binary->left->kind == NODE_KIND_LT)
		{
			result = syntax_relational(syntax, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_binary->left->kind == NODE_KIND_LE)
		{
			result = syntax_relational(syntax, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_binary->left->kind == NODE_KIND_GT)
		{
			result = syntax_relational(syntax, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_binary->left->kind == NODE_KIND_GE)
		{
			result = syntax_relational(syntax, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else
		{
			result = syntax_shifting(syntax, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}

		result = syntax_shifting(syntax, node, node_binary->right);
		if (!result)
		{
			return 0;
		}
		break;

	default:
		result = syntax_shifting(syntax, parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}

	return 1;
}

static int32_t
syntax_equality(syntax_t *syntax, node_t *parent, node_t *node)
{
	int32_t result;
	node_binary_t *node_binary;
	switch (node->kind)
	{
	case NODE_KIND_EQ:
	case NODE_KIND_IN:
	case NODE_KIND_EXTENDS:
	case NODE_KIND_NEQ:
		node->parent = parent;
		node_binary = (node_binary_t *)node->value;
		if (node_binary->left->kind == NODE_KIND_IN)
		{
			result = syntax_equality(syntax, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_binary->left->kind == NODE_KIND_EQ)
		{
			result = syntax_equality(syntax, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_binary->left->kind == NODE_KIND_NEQ)
		{
			result = syntax_equality(syntax, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else
		{
			result = syntax_relational(syntax, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}

		result = syntax_relational(syntax, node, node_binary->right);
		if (!result)
		{
			return 0;
		}
		break;

	default:
		result = syntax_relational(syntax, parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}

	return 1;
}

static int32_t
syntax_bitwise_and(syntax_t *syntax, node_t *parent, node_t *node)
{
	int32_t result;
	node_binary_t *node_binary;
	switch (node->kind)
	{
	case NODE_KIND_AND:
		node->parent = parent;
		node_binary = (node_binary_t *)node->value;
		if (node_binary->left->kind == NODE_KIND_AND)
		{
			result = syntax_bitwise_and(syntax, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else
		{
			result = syntax_equality(syntax, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}

		result = syntax_equality(syntax, node, node_binary->right);
		if (!result)
		{
			return 0;
		}
		break;

	default:
		result = syntax_equality(syntax, parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}

	return 1;
}

static int32_t
syntax_bitwise_xor(syntax_t *syntax, node_t *parent, node_t *node)
{
	int32_t result;
	node_binary_t *node_binary;
	switch (node->kind)
	{
	case NODE_KIND_XOR:
		node->parent = parent;
		node_binary = (node_binary_t *)node->value;
		if (node_binary->left->kind == NODE_KIND_XOR)
		{
			result = syntax_bitwise_xor(syntax, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else
		{
			result = syntax_bitwise_and(syntax, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}

		result = syntax_bitwise_and(syntax, node, node_binary->right);
		if (!result)
		{
			return 0;
		}
		break;

	default:
		result = syntax_bitwise_and(syntax, parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}

	return 1;
}

static int32_t
syntax_bitwise_or(syntax_t *syntax, node_t *parent, node_t *node)
{
	int32_t result;
	node_binary_t *node_binary;
	switch (node->kind)
	{
	case NODE_KIND_OR:
		node->parent = parent;
		node_binary = (node_binary_t *)node->value;
		if (node_binary->left->kind == NODE_KIND_OR)
		{
			result = syntax_bitwise_or(syntax, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else
		{
			result = syntax_bitwise_xor(syntax, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}

		result = syntax_bitwise_xor(syntax, node, node_binary->right);
		if (!result)
		{
			return 0;
		}
		break;

	default:
		result = syntax_bitwise_xor(syntax, parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}

	return 1;
}

static int32_t
syntax_logical_and(syntax_t *syntax, node_t *parent, node_t *node)
{
	int32_t result;
	node_binary_t *node_binary;
	switch (node->kind)
	{
	case NODE_KIND_LAND:
		node->parent = parent;
		node_binary = (node_binary_t *)node->value;
		if (node_binary->left->kind == NODE_KIND_LAND)
		{
			result = syntax_logical_and(syntax, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else
		{
			result = syntax_bitwise_or(syntax, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}

		result = syntax_bitwise_or(syntax, node, node_binary->right);
		if (!result)
		{
			return 0;
		}
		break;

	default:
		result = syntax_bitwise_or(syntax, parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}

	return 1;
}

static int32_t
syntax_logical_or(syntax_t *syntax, node_t *parent, node_t *node)
{
	int32_t result;
	node_binary_t *node_binary;
	switch (node->kind)
	{
	case NODE_KIND_LOR:
		node->parent = parent;
		node_binary = (node_binary_t *)node->value;
		if (node_binary->left->kind == NODE_KIND_LOR)
		{
			result = syntax_logical_or(syntax, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else
		{
			result = syntax_logical_and(syntax, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}

		result = syntax_logical_and(syntax, node, node_binary->right);
		if (!result)
		{
			return 0;
		}
		break;

	default:
		result = syntax_logical_and(syntax, parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}

	return 1;
}

static int32_t
syntax_conditional(syntax_t *syntax, node_t *parent, node_t *node)
{
	int32_t result;
	node_conditional_t *node_conditional;
	switch (node->kind)
	{
	case NODE_KIND_CONDITIONAL:
		node->parent = parent;
		node_conditional = (node_conditional_t *)node->value;
		result = syntax_logical_or(syntax, node, node_conditional->condition);
		if (!result)
		{
			return 0;
		}
		result = syntax_conditional(syntax, node, node_conditional->true_expression);
		if (!result)
		{
			return 0;
		}
		result = syntax_conditional(syntax, node, node_conditional->false_expression);
		if (!result)
		{
			return 0;
		}
		break;

	default:
		result = syntax_logical_or(syntax, parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}

	return 1;
}

static int32_t
syntax_expression(syntax_t *syntax, node_t *parent, node_t *node)
{
	return syntax_conditional(syntax, parent, node);
}

static int32_t
syntax_assign(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_binary_t *node_binary;

	switch (node->kind)
	{
	case NODE_KIND_ASSIGN:
	case NODE_KIND_DEFINE:
	case NODE_KIND_ADD_ASSIGN:
	case NODE_KIND_SUB_ASSIGN:
	case NODE_KIND_DIV_ASSIGN:
	case NODE_KIND_MUL_ASSIGN:
	case NODE_KIND_MOD_ASSIGN:
	case NODE_KIND_AND_ASSIGN:
	case NODE_KIND_OR_ASSIGN:
	case NODE_KIND_SHL_ASSIGN:
	case NODE_KIND_SHR_ASSIGN:
		node_binary = (node_binary_t *)node->value;

		int32_t result;
		result = syntax_expression(syntax, node, node_binary->left);
		if (!result)
		{
			return 0;
		}

		result = syntax_expression(syntax, node, node_binary->right);
		if (!result)
		{
			return 0;
		}
		break;

	default:
		result = syntax_expression(syntax, parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}

	return 1;
}

static int32_t
syntax_if(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_if_t *node_if;
	node_if = (node_if_t *)node->value;

	int32_t result;
	result = syntax_expression(syntax, node, node_if->condition);
	if (!result)
	{
		return 0;
	}

	result = syntax_block(syntax, node, node_if->then_body);
	if (!result)
	{
		return 0;
	}

	if (node_if->else_body)
	{
		switch (node_if->else_body->kind)
		{
		case NODE_KIND_IF:
			result = syntax_if(syntax, node, node_if->else_body);
			break;

		default:
			result = syntax_block(syntax, node, node_if->else_body);
			break;
		}
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
syntax_catch(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_catch_t *node_catch;
	node_catch = (node_catch_t *)node->value;

	int32_t result;
	if (node_catch->parameter)
	{
		result = syntax_parameter(syntax, node, node_catch->parameter);
		if (!result)
		{
			return 0;
		}
	}

	result = syntax_block(syntax, node, node_catch->body);
	if (!result)
	{
		return 0;
	}

	return 1;
}

static int32_t
syntax_try(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_try_t *node_try;
	node_try = (node_try_t *)node->value;

	int32_t result;
	result = syntax_block(syntax, node, node_try->body);
	if (!result)
	{
		return 0;
	}

	ilist_t *a;
	for (a = node_try->catchs->begin; a != node_try->catchs->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;
		result = syntax_catch(syntax, node, temp);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
syntax_for_init(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_enumerable_t *node_enumerable;
	node_enumerable = (node_enumerable_t *)node->value;

	int32_t result;
	ilist_t *a;
	for (a = node_enumerable->list->begin; a != node_enumerable->list->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		switch (temp->kind)
		{
		case NODE_KIND_VAR:
			result = syntax_var(syntax, node, temp);
			break;

		case NODE_KIND_CONST:
			result = syntax_const(syntax, node, temp);
			break;

		default:
			result = syntax_assign(syntax, node, temp);
			break;
		}

		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
syntax_for_step(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_enumerable_t *node_enumerable;
	node_enumerable = (node_enumerable_t *)node->value;

	int32_t result;
	ilist_t *a;
	for (a = node_enumerable->list->begin; a != node_enumerable->list->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		result = syntax_assign(syntax, node, temp);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
syntax_for(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_for_t *node_for;
	node_for = (node_for_t *)node->value;

	int32_t result;
	if (node_for->initializer)
	{
		switch (node_for->initializer->kind)
		{
		case NODE_KIND_FOR_INIT_LIST:
			result = syntax_for_init(syntax, node, node_for->initializer);
			break;

		case NODE_KIND_VAR:
			result = syntax_var(syntax, node, node_for->initializer);
			break;

		case NODE_KIND_CONST:
			result = syntax_const(syntax, node, node_for->initializer);
			break;

		default:
			result = syntax_assign(syntax, node, node_for->initializer);
			break;
		}
	}

	if (node_for->condition)
	{
		result = syntax_expression(syntax, node, node_for->condition);
		if (!result)
		{
			return 0;
		}
	}

	if (node_for->incrementor)
	{
		switch (node_for->incrementor->kind)
		{
		case NODE_KIND_FOR_STEP_LIST:
			result = syntax_for_step(syntax, node, node_for->incrementor);
			break;

		default:
			result = syntax_assign(syntax, node, node_for->incrementor);
			break;
		}

		if (!result)
		{
			return 0;
		}
	}

	result = syntax_block(syntax, node, node_for->body);
	if (!result)
	{
		return 0;
	}

	return 1;
}

static int32_t
syntax_forin(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_forin_t *node_forin;
	node_forin = (node_forin_t *)node->value;

	int32_t result;
	switch (node_forin->initializer->kind)
	{
	case NODE_KIND_VAR:
		result = syntax_var(syntax, node, node_forin->initializer);
		break;

	case NODE_KIND_CONST:
		result = syntax_const(syntax, node, node_forin->initializer);
		break;

	default:
		syntax_error(syntax, node->position, "unknown initializer for..in loop");
		return 0;
	}
	if (!result)
	{
		return 0;
	}

	result = syntax_expression(syntax, node, node_forin->expression);
	if (!result)
	{
		return 0;
	}

	result = syntax_block(syntax, node, node_forin->body);
	if (!result)
	{
		return 0;
	}

	return 1;
}

static int32_t
syntax_break(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;
	return 1;
}

static int32_t
syntax_continue(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;
	return 1;
}

static int32_t
syntax_return(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_return_t *node_return;
	node_return = (node_return_t *)node->value;

	int32_t result = syntax_expression(syntax, node, node_return->expression);
	if (!result)
	{
		return 0;
	}

	return 1;
}

static int32_t
syntax_throw(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_throw_t *node_throw;
	node_throw = (node_throw_t *)node->value;

	int32_t result = syntax_expression(syntax, node, node_throw->expression);
	if (!result)
	{
		return 0;
	}

	return 1;
}

static int32_t
syntax_statement(syntax_t *syntax, node_t *parent, node_t *node)
{
	int32_t result = 0;
	switch (node->kind)
	{
	case NODE_KIND_BLOCK:
		result = syntax_block(syntax, parent, node);
		break;

	case NODE_KIND_IF:
		result = syntax_if(syntax, parent, node);
		break;

	case NODE_KIND_TRY:
		result = syntax_try(syntax, parent, node);
		break;

	case NODE_KIND_FOR:
		result = syntax_for(syntax, parent, node);
		break;

	case NODE_KIND_FORIN:
		result = syntax_forin(syntax, parent, node);
		break;

	case NODE_KIND_VAR:
		result = syntax_var(syntax, parent, node);
		break;

	case NODE_KIND_CONST:
		result = syntax_const(syntax, parent, node);
		break;

	case NODE_KIND_TYPE:
		result = syntax_type(syntax, parent, node);
		break;

	case NODE_KIND_BREAK:
		result = syntax_break(syntax, parent, node);
		break;

	case NODE_KIND_CONTINUE:
		result = syntax_continue(syntax, parent, node);
		break;

	case NODE_KIND_RETURN:
		result = syntax_return(syntax, parent, node);
		break;

	case NODE_KIND_THROW:
		result = syntax_throw(syntax, parent, node);
		break;

	case NODE_KIND_FUNC:
		result = syntax_func(syntax, parent, node);
		break;

	default:
		result = syntax_assign(syntax, parent, node);
		break;
	}

	if (!result)
	{
		return 0;
	}

	return 1;
}

static int32_t
syntax_block(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_block_t *node_block;
	node_block = (node_block_t *)node->value;

	int32_t result;
	ilist_t *a;
	for (a = node_block->list->begin; a != node_block->list->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		result = syntax_statement(syntax, node, temp);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
syntax_check_is_type(syntax_t *syntax, node_t *node);

static int32_t
syntax_type_parameter(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_type_parameter_t *node_type_parameter;
	node_type_parameter = (node_type_parameter_t *)node->value;

	int32_t result;
	result = syntax_id(syntax, node, node_type_parameter->name);
	if (!result)
	{
		return 0;
	}

	if (node_type_parameter->extends)
	{
		syntax_check_is_type(syntax, node_type_parameter->extends);

		result = syntax_expression(syntax, node, node_type_parameter->extends);
		if (!result)
		{
			return 0;
		}
	}

	if (node_type_parameter->value)
	{
		syntax_check_is_type(syntax, node_type_parameter->value);

		result = syntax_expression(syntax, node, node_type_parameter->value);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
syntax_enum_member(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_enum_member_t *node_enum_member;
	node_enum_member = (node_enum_member_t *)node->value;

	int32_t result;
	result = syntax_id(syntax, node, node_enum_member->name);
	if (!result)
	{
		return 0;
	}

	if (node_enum_member->value)
	{
		result = syntax_expression(syntax, node, node_enum_member->value);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
syntax_enum(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_enum_t *node_enum;
	node_enum = (node_enum_t *)node->value;

	int32_t result;
	result = syntax_id(syntax, node, node_enum->name);
	if (!result)
	{
		return 0;
	}

	ilist_t *a;
	for (a = node_enum->body->begin; a != node_enum->body->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		result = syntax_enum_member(syntax, node, temp);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
syntax_property(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_property_t *node_property;
	node_property = (node_property_t *)node->value;

	int32_t result;
	result = syntax_id(syntax, node, node_property->name);
	if (!result)
	{
		return 0;
	}

	if (node_property->type)
	{
		result = syntax_expression(syntax, node, node_property->type);
		if (!result)
		{
			return 0;
		}
	}

	if (node_property->value)
	{
		result = syntax_expression(syntax, node, node_property->value);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
syntax_check_is_object_variable(syntax_t *syntax, node_t *node);

static int32_t
syntax_check_is_get_attr(syntax_t *syntax, node_t *node)
{
	int32_t result = 0;

	if (node->kind == NODE_KIND_GET_ATTR)
	{
		node_get_attr_t *node_get_attr;
		node_get_attr = (node_get_attr_t *)node->value;

		result |= syntax_check_is_get_attr(syntax, node_get_attr->left);
		result |= syntax_check_is_get_attr(syntax, node_get_attr->right);
	}
	else if (node->kind != NODE_KIND_ID)
	{
		syntax_error(syntax, node->position, "expected identifire");
		return 0;
	}

	if(result == 0){
		return 0;
	}
	return 1;
}

static int32_t
syntax_check_is_object_variable_value(syntax_t *syntax, node_t *node)
{
	int32_t result = 0;

	if (node->kind == NODE_KIND_OBJECT)
	{
		result |= syntax_check_is_object_variable(syntax, node);
	}
	else if (node->kind == NODE_KIND_GET_ATTR)
	{
		result |= syntax_check_is_get_attr(syntax, node);
	}
	else if (node->kind != NODE_KIND_ID)
	{
		syntax_error(syntax, node->position, "expected rhs variable");
		return 0;
	}

	return 1;
}

static int32_t
syntax_check_is_object_variable_name(syntax_t *syntax, node_t *node)
{
	int32_t result = 0;

	if (node->kind == NODE_KIND_OBJECT)
	{
		result |= syntax_check_is_object_variable(syntax, node);
	}
	else if (node->kind != NODE_KIND_ID)
	{
		syntax_error(syntax, node->position, "expected variable name");
		return 0;
	}

	if(result == 0){
		return 0;
	}
	return 1;
}

static int32_t
syntax_check_is_object_property_variable(syntax_t *syntax, node_t *node)
{
	int32_t result = 0;

	node_object_property_t *node_object_property;
	node_object_property = (node_object_property_t *)node->value;

	if (node_object_property->name)
	{
		result |= syntax_check_is_object_variable_name(syntax, node_object_property->name);
	}

	if (node_object_property->value)
	{
		result |= syntax_check_is_object_variable_value(syntax, node_object_property->value);
	}

	if(result == 0){
		return 0;
	}
	return 1;
}

static int32_t
syntax_check_is_object_variable(syntax_t *syntax, node_t *node)
{
	int32_t result = 0;

	node_object_t *node_object;
	node_object = (node_object_t *)node->value;

	ilist_t *a;
	for (a = node_object->list->begin; a != node_object->list->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;
		result |= syntax_check_is_object_property_variable(syntax, temp);
	}

	if(result == 0){
		return 0;
	}
	return 1;
}

static int32_t
syntax_check_is_variable_name(syntax_t *syntax, node_t *node)
{
	if (node->kind != NODE_KIND_ID)
	{
		syntax_error(syntax, node->position, "expected variable name");
		return 0;
	}
	return 1;
}

static int32_t
syntax_check_is_array_variable(syntax_t *syntax, node_t *node)
{
	int32_t result = 0;

	node_array_t *node_array;
	node_array = (node_array_t *)node->value;

	ilist_t *a;
	for (a = node_array->list->begin; a != node_array->list->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		result |= syntax_check_is_variable_name(syntax, temp);
	}

	if(result == 0){
		return 0;
	}
	return 1;
}

static int32_t
syntax_check_is_ellipsisiable(syntax_t *syntax, node_t *node)
{
	int32_t result = 0;

	if (node->kind == NODE_KIND_ARRAY)
	{
		result |= syntax_check_is_array_variable(syntax, node);
	}
	else if (node->kind != NODE_KIND_ID)
	{
		syntax_error(syntax, node->position, "expected identifier or array variable");
		return 0;
	}

	if(result == 0){
		return 0;
	}
	return 1;
}

static int32_t
syntax_check_is_parameter_ellipsis(syntax_t *syntax, node_t *node)
{
	int32_t result = 0;

	node_unary_t *node_unary;
	node_unary = (node_unary_t *)node->value;

	result |= syntax_check_is_ellipsisiable(syntax, node_unary->right);

	if(result == 0){
		return 0;
	}
	return 1;
}

static int32_t
syntax_check_is_parameter_name(syntax_t *syntax, node_t *node)
{
	int32_t result = 0;

	if (node->kind == NODE_KIND_OBJECT)
	{
		result |= syntax_check_is_object_variable(syntax, node);
	}
	else if (node->kind == NODE_KIND_ARRAY)
	{
		result |= syntax_check_is_array_variable(syntax, node);
	}
	else if (node->kind == NODE_KIND_ELLIPSIS)
	{
		result |= syntax_check_is_parameter_ellipsis(syntax, node);
	}
	else if (node->kind != NODE_KIND_ID)
	{
		syntax_error(syntax, node->position, "expected parameter");
		return 0;
	}

	if(result == 0){
		return 0;
	}
	return 1;
}

static int32_t
syntax_check_is_type_array(syntax_t *syntax, node_t *node)
{
	int32_t result = 0;

	node_array_t *node_array;
	node_array = (node_array_t *)node->value;

	ilist_t *a;
	for (a = node_array->list->begin; a != node_array->list->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		result |= syntax_check_is_type(syntax, temp);
	}

	if(result == 0){
		return 0;
	}
	return 1;
}

static int32_t
syntax_check_is_type_object_key(syntax_t *syntax, node_t *node)
{
	if (
			(node->kind != NODE_KIND_ID) &&
			(node->kind != NODE_KIND_STRING) &&
			(node->kind != NODE_KIND_NUMBER) &&
			(node->kind != NODE_KIND_CHAR))
	{
		syntax_error(syntax, node->position, "expected identifier");
		return 0;
	}

	return 1;
}

static int32_t
syntax_check_is_type_object_property(syntax_t *syntax, node_t *node)
{
	int32_t result = 0;

	node_object_property_t *node_object_property;
	node_object_property = (node_object_property_t *)node->value;

	result |= syntax_check_is_type_object_key(syntax, node_object_property->name);

	if (node_object_property->value)
	{
		result |= syntax_check_is_type(syntax, node_object_property->value);
	}

	if(result == 0){
		return 0;
	}
	return 1;
}

static int32_t
syntax_check_is_type_object(syntax_t *syntax, node_t *node)
{
	int32_t result = 0;

	node_object_t *node_object;
	node_object = (node_object_t *)node->value;

	ilist_t *a;
	for (a = node_object->list->begin; a != node_object->list->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		result |= syntax_check_is_type_object_property(syntax, temp);
	}

	if(result == 0){
		return 0;
	}
	return 1;
}

static int32_t
syntax_check_is_type_func(syntax_t *syntax, node_t *node)
{
	node_func_t *node_func;
	node_func = (node_func_t *)node->value;
	
	if (node_func->fields)
	{
		syntax_error(syntax, node->position, "no fields in func type");
		return 0;
	}
	if (node_func->name)
	{
		syntax_error(syntax, node->position, "no name in func type");
		return 0;
	}
	if (node_func->body)
	{
		syntax_error(syntax, node->position, "no body in func type");
		return 0;
	}

	return 1;
}

static int32_t
syntax_check_is_type_parenthesis(syntax_t *syntax, node_t *node)
{
	int32_t result = 0;

	node_unary_t *node_unary;
	node_unary = (node_unary_t *)node->value;

	result |= syntax_check_is_type(syntax, node_unary->right);

	if(result == 0){
		return 0;
	}
	return 1;
}

static int32_t
syntax_check_is_type_primary(syntax_t *syntax, node_t *node)
{
	int32_t result = 0;
	switch (node->kind)
	{
	case NODE_KIND_ID:
	case NODE_KIND_NUMBER:
	case NODE_KIND_CHAR:
	case NODE_KIND_STRING:
	case NODE_KIND_NULL:
	case NODE_KIND_TRUE:
	case NODE_KIND_FALSE:
		break;

	case NODE_KIND_ARRAY:
		result |= syntax_check_is_type_array(syntax, node);
		break;

	case NODE_KIND_OBJECT:
		result |= syntax_check_is_type_object(syntax, node);
		break;

	case NODE_KIND_FUNC:
		result |= syntax_check_is_type_func(syntax, node);
		break;

	case NODE_KIND_PARENTHESIS:
		result |= syntax_check_is_type_parenthesis(syntax, node);
		break;

	default:
		syntax_error(syntax, node->position, "expected type");
		break;
	}

	if(result == 0){
		return 0;
	}
	return 1;
}

static int32_t
syntax_check_is_type_composite(syntax_t *syntax, node_t *node)
{
	int32_t result = 0;

	node_composite_t *node_composite;
	node_composite = (node_composite_t *)node->value;

	result |= syntax_check_is_name(syntax, node_composite->base);

	if (node_composite->type_arguments)
	{
		ilist_t *a;
		for (a = node_composite->type_arguments->begin; a != node_composite->type_arguments->end; a = a->next)
		{
			node_t *temp;
			temp = (node_t *)a->value;

			result |= syntax_check_is_type(syntax, temp);
		}
	}

	if(result == 0){
		return 0;
	}
	return 1;
}

static int32_t
syntax_check_is_type_postfix(syntax_t *syntax, node_t *node)
{
	int32_t result = 0;

	switch (node->kind)
	{
	case NODE_KIND_COMPOSITE:
		result |= syntax_check_is_type_composite(syntax, node);
		break;

	case NODE_KIND_CALL:
	case NODE_KIND_GET_SLICE:
		syntax_error(syntax, node->position, "expected type");
		return 0;

	case NODE_KIND_GET_ITEM:
	case NODE_KIND_GET_ATTR:
		break;

	default:
		result |= syntax_check_is_type_primary(syntax, node);
		break;
	}

	if(result == 0){
		return 0;
	}
	return 1;
}

static int32_t
syntax_check_is_type_unary(syntax_t *syntax, node_t *node)
{
	int32_t result = 0;

	node_unary_t *node_unary;
	switch (node->kind)
	{
	case NODE_KIND_TYPEOF:
	case NODE_KIND_GET_VALUE:
	case NODE_KIND_GET_ADDRESS:
		node_unary = (node_unary_t *)node->value;
		result |= syntax_check_is_type_unary(syntax, node_unary->right);
		break;

	case NODE_KIND_TILDE:
	case NODE_KIND_NOT:
	case NODE_KIND_NEG:
	case NODE_KIND_POS:
	case NODE_KIND_SIZEOF:
	case NODE_KIND_ELLIPSIS:
		syntax_error(syntax, node->position, "expected type");
		return 0;

	default:
		result |= syntax_check_is_type_postfix(syntax, node);
		break;
	}

	if(result == 0){
		return 0;
	}
	return 1;
}

static int32_t
syntax_check_is_type_binary(syntax_t *syntax, node_t *node)
{
	int32_t result = 0;
	node_binary_t *node_binary;

	switch (node->kind)
	{
	case NODE_KIND_LOR:
	case NODE_KIND_LAND:	
	case NODE_KIND_XOR:
	case NODE_KIND_IN:
	case NODE_KIND_GE:
	case NODE_KIND_GT:
	case NODE_KIND_LE:
	case NODE_KIND_LT:
	case NODE_KIND_SHR:
	case NODE_KIND_SHL:
	case NODE_KIND_MINUS:
	case NODE_KIND_PLUS:
	case NODE_KIND_MOD:
	case NODE_KIND_DIV:
	case NODE_KIND_MUL:
		syntax_error(syntax, node->position, "expected type operator");
		return 0;

	case NODE_KIND_OR:
	case NODE_KIND_AND:
	case NODE_KIND_NEQ:
	case NODE_KIND_EQ:
	case NODE_KIND_EXTENDS:
		node_binary = (node_binary_t *)node->value;
		result |= syntax_check_is_type_binary(syntax, node_binary->left);
		result |= syntax_check_is_type_binary(syntax, node_binary->right);
		break;

	default:
		result |= syntax_check_is_type_unary(syntax, node);
		break;
	}

	if(result == 0){
		return 0;
	}
	return 1;
}

static int32_t
syntax_check_is_type_conditional(syntax_t *syntax, node_t *node)
{
	int32_t result = 0;
	if (node->kind == NODE_KIND_CONDITIONAL)
	{
		node_conditional_t *node_conditional;
		node_conditional = (node_conditional_t *)node->value;

		result |= syntax_check_is_type_binary(syntax, node_conditional->condition);
		result |= syntax_check_is_type_conditional(syntax, node_conditional->true_expression);
		result |= syntax_check_is_type_conditional(syntax, node_conditional->false_expression);
	}
	else
	{
		result |= syntax_check_is_type_binary(syntax, node);
	}

	if(result == 0){
		return 0;
	}
	return 1;
}

static int32_t
syntax_check_is_type(syntax_t *syntax, node_t *node)
{
	return syntax_check_is_type_conditional(syntax, node);
}

static int32_t
syntax_parameter(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_parameter_t *node_parameter;
	node_parameter = (node_parameter_t *)node->value;

	syntax_check_is_parameter_name(syntax, node_parameter->name);

	int32_t result;
	result = syntax_expression(syntax, node, node_parameter->name);
	if (!result)
	{
		return 0;
	}

	if (node_parameter->type)
	{
		syntax_check_is_type(syntax, node_parameter->type);

		result = syntax_expression(syntax, node, node_parameter->type);
		if (!result)
		{
			return 0;
		}
	}

	if (node_parameter->value)
	{
		syntax_check_is_type(syntax, node_parameter->value);

		result = syntax_expression(syntax, node, node_parameter->value);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
syntax_check_is_name(syntax_t *syntax, node_t *node)
{
	if (node->kind != NODE_KIND_ID)
	{
		return 0;
	}
	return 1;
}

static int32_t
syntax_check_is_hierarchy_name(syntax_t *syntax, node_t *node)
{
	int32_t result;
	if (node->kind == NODE_KIND_GET_ATTR)
	{
		node_get_attr_t *node_get_attr;
		node_get_attr = (node_get_attr_t *)node->value;
		result = syntax_check_is_hierarchy_name(syntax, node_get_attr->left);
		if (!result)
		{
			return 0;
		}
		result = syntax_check_is_name(syntax, node_get_attr->right);
		if (!result)
		{
			return 0;
		}
	}
	else if (node->kind != NODE_KIND_ID)
	{
		return 0;
	}
	return 1;
}

static int32_t
syntax_field(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_field_t *node_field;
	node_field = (node_field_t *)node->value;

	int32_t result;
	result = syntax_id(syntax, node, node_field->name);
	if (!result)
	{
		return 0;
	}

	if(node_field->type)
	{
			syntax_check_is_type(syntax, node_field->type);
			syntax_check_is_hierarchy_name(syntax, node_field->type);

			result = syntax_expression(syntax, node, node_field->type);
			if (!result)
			{
				return 0;
			}
	}

	return 1;
}

static int32_t
syntax_method(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_method_t *node_method;
	node_method = (node_method_t *)node->value;

	int32_t result;
	result = syntax_id(syntax, node, node_method->name);
	if (!result)
	{
		return 0;
	}

	ilist_t *a;
	if (node_method->parameters)
	{
		for (a = node_method->parameters->begin; a != node_method->parameters->end; a = a->next)
		{
			node_t *temp;
			temp = (node_t *)a->value;

			result = syntax_parameter(syntax, node, temp);
			if (!result)
			{
				return 0;
			}
		}
	}

	if (node_method->type_parameters)
	{
		for (a = node_method->type_parameters->begin; a != node_method->type_parameters->end; a = a->next)
		{
			node_t *temp;
			temp = (node_t *)a->value;

			result = syntax_type_parameter(syntax, node, temp);
			if (!result)
			{
				return 0;
			}
		}
	}

	if(node_method->body)
	{
		if(node_method->body->kind == NODE_KIND_BLOCK)
		{
			result = syntax_block(syntax, node, node_method->body);
		}
		else
		{
			result = syntax_expression(syntax, node, node_method->body);
		}
	}
	
	if (!result)
	{
		return 0;
	}

	return 1;
}

static int32_t
syntax_check_is_hierarchy_attribute(syntax_t *syntax, node_t *node)
{
	int32_t result;
	if (node->kind == NODE_KIND_GET_ATTR)
	{
		node_get_attr_t *node_get_attr;
		node_get_attr = (node_get_attr_t *)node->value;

		result = syntax_check_is_hierarchy_attribute(syntax, node_get_attr->left);
		if (!result)
		{
			return 0;
		}

		if(node_get_attr->right->kind == NODE_KIND_COMPOSITE)
		{
			result = syntax_check_is_type_composite(syntax, node_get_attr->right);
			if (!result)
			{
				return 0;
			}
		}
		else
		{
			result = syntax_check_is_name(syntax, node_get_attr->right);
			if (!result)
			{
				return 0;
			}
		}
	}
	if (node->kind == NODE_KIND_COMPOSITE)
	{
		result = syntax_check_is_type_composite(syntax, node);
		if (!result)
		{
			return 0;
		}
	}
	else if (node->kind != NODE_KIND_ID)
	{
		return 0;
	}
	return 1;
}

static int32_t
syntax_heritage(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_heritage_t *node_heritage;
	node_heritage = (node_heritage_t *)node->value;

	int32_t result;
	result = syntax_id(syntax, node, node_heritage->name);
	if (!result)
	{
		return 0;
	}

	if(node_heritage->type)
	{
		result = syntax_expression(syntax, node, node_heritage->type);
		if (!result)
		{
			return 0;
		}
		result = syntax_check_is_hierarchy_attribute(syntax, node_heritage->type);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
syntax_class(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_class_t *node_class;
	node_class = (node_class_t *)node->value;

	int32_t result;
	result = syntax_id(syntax, node, node_class->name);
	if (!result)
	{
		return 0;
	}

	ilist_t *a;
	if (node_class->heritages)
	{
		for (a = node_class->heritages->begin; a != node_class->heritages->end; a = a->next)
		{
			node_t *temp;
			temp = (node_t *)a->value;

			result = syntax_heritage(syntax, node, temp);
			if (!result)
			{
				return 0;
			}
		}
	}

	if (node_class->type_parameters)
	{
		for (a = node_class->type_parameters->begin; a != node_class->type_parameters->end; a = a->next)
		{
			node_t *temp;
			temp = (node_t *)a->value;

			result = syntax_type_parameter(syntax, node, temp);
			if (!result)
			{
				return 0;
			}
		}
	}

	for (a = node_class->body->begin; a != node_class->body->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;
		
		switch (temp->kind)
		{
		case NODE_KIND_EXPORT:
			result = syntax_export(syntax, node, temp);
			if (!result)
			{
				return 0;
			}
			break;

		case NODE_KIND_CLASS:
			result = syntax_class(syntax, node, temp);
			if (!result)
			{
				return 0;
			}
			break;

		case NODE_KIND_ENUM:
			result = syntax_enum(syntax, node, temp);
			if (!result)
			{
				return 0;
			}
			break;

		case NODE_KIND_PROPERTY:
			result = syntax_property(syntax, node, temp);
			if (!result)
			{
				return 0;
			}
			break;

		case NODE_KIND_METHOD:
			result = syntax_method(syntax, node, temp);
			if (!result)
			{
				return 0;
			}
			break;

		default:
			syntax_error(syntax, node->position, "unknown node body decalaration");
			return 0;
		}
	}

	return 1;
}

static int32_t
syntax_func(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_func_t *node_func;
	node_func = (node_func_t *)node->value;

	int32_t result;
	if (node_func->name)
	{
		result = syntax_id(syntax, node, node_func->name);
		if (!result)
		{
			return 0;
		}
	}

	ilist_t *a;
	if (node_func->parameters)
	{
		for (a = node_func->parameters->begin; a != node_func->parameters->end; a = a->next)
		{
			node_t *temp;
			temp = (node_t *)a->value;

			result = syntax_parameter(syntax, node, temp);
			if (!result)
			{
				return 0;
			}
		}
	}

	if (node_func->type_parameters)
	{
		for (a = node_func->type_parameters->begin; a != node_func->type_parameters->end; a = a->next)
		{
			node_t *temp;
			temp = (node_t *)a->value;

			result = syntax_type_parameter(syntax, node, temp);
			if (!result)
			{
				return 0;
			}
		}
	}

	if (node_func->fields)
	{
		ilist_t *a;
		for (a = node_func->fields->begin; a != node_func->fields->end; a = a->next)
		{
			node_t *temp;
			temp = (node_t *)a->value;
			result = syntax_field(syntax, node, temp);
			if (!result)
			{
				return 0;
			}
		}
	}

	if (node_func->body)
	{
		if(node_func->body->kind == NODE_KIND_BLOCK)
		{
			result = syntax_block(syntax, node, node_func->body);
		}
		else
		{
			result = syntax_expression(syntax, node, node_func->body);
		}
		
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static void
syntax_check_is_var_object_property(syntax_t *syntax, node_t *node)
{
	node_object_property_t *node_object_property;
	node_object_property = (node_object_property_t *)node->value;

	int32_t result;
	result = syntax_check_is_name(syntax, node_object_property->name);
	if (!result)
	{
		if (node_object_property->value)
		{
			syntax_error(syntax, node_object_property->name->position,
										"The name must be an identifier");
		}
		else
		{
			syntax_error(syntax, node_object_property->name->position,
										"Valuation must be with field name of object or nested object");
		}
	}
	if (node_object_property->value)
	{
		result = syntax_check_is_hierarchy_name(syntax, node_object_property->value);
		if (!result)
		{
			syntax_error(syntax, node_object_property->value->position,
										"Valuation must be with identifier of object or nested object");
		}
	}
}

static void
syntax_check_is_var_object(syntax_t *syntax, node_t *node)
{
	node_object_t *node_object;
	node_object = (node_object_t *)node->value;

	ilist_t *a;
	for (a = node_object->list->begin; a != node_object->list->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		syntax_check_is_var_object_property(syntax, temp);
	}
}

static void
syntax_check_is_var_array(syntax_t *syntax, node_t *node)
{
	node_array_t *node_array;
	node_array = (node_array_t *)node->value;

	ilist_t *a;
	for (a = node_array->list->begin; a != node_array->list->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		int32_t result;
		result = syntax_check_is_name(syntax, temp);
		if (!result)
		{
			syntax_error(syntax, temp->position,
										"In valuing by array, the fields must include the variable name");
		}
	}
}

static int32_t
syntax_var(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_var_t *node_var;
	node_var = (node_var_t *)node->value;

	int32_t result;
	if (node_var->name->kind == NODE_KIND_OBJECT)
	{
		syntax_check_is_var_object(syntax, node_var->name);
		result = syntax_object(syntax, node, node_var->name);
		if (!result)
		{
			return 0;
		}
	}
	else if (node_var->name->kind == NODE_KIND_ARRAY)
	{
		syntax_check_is_var_array(syntax, node_var->name);
		result = syntax_array(syntax, node, node_var->name);
		if (!result)
		{
			return 0;
		}
	}
	else
	{
		result = syntax_id(syntax, node, node_var->name);
		if (!result)
		{
			return 0;
		}
	}

	if (node_var->type)
	{
		result = syntax_expression(syntax, node, node_var->type);
		if (!result)
		{
			return 0;
		}
	}
	if (node_var->value)
	{
		result = syntax_expression(syntax, node, node_var->value);
		if (!result)
		{
			return 0;
		}
	}
	return 1;
}

static int32_t
syntax_const(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_const_t *node_const;
	node_const = (node_const_t *)node->value;

	int32_t result;
	if (node_const->name->kind == NODE_KIND_OBJECT)
	{
		syntax_check_is_var_object(syntax, node_const->name);
		result = syntax_object(syntax, node, node_const->name);
		if (!result)
		{
			return 0;
		}
	}
	else if (node_const->name->kind == NODE_KIND_ARRAY)
	{
		syntax_check_is_var_array(syntax, node_const->name);
		result = syntax_array(syntax, node, node_const->name);
		if (!result)
		{
			return 0;
		}
	}
	else
	{
		result = syntax_id(syntax, node, node_const->name);
		if (!result)
		{
			return 0;
		}
	}

	if (node_const->type)
	{
		result = syntax_expression(syntax, node, node_const->type);
		if (!result)
		{
			return 0;
		}
	}
	if (node_const->value)
	{
		result = syntax_expression(syntax, node, node_const->value);
		if (!result)
		{
			return 0;
		}
	}
	return 1;
}

static int32_t
syntax_type(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_type_t *node_type;
	node_type = (node_type_t *)node->value;

	int32_t result;
	result = syntax_id(syntax, node, node_type->name);
	if (!result)
	{
		return 0;
	}

	ilist_t *a;
	if (node_type->type_parameters)
	{
		for (a = node_type->type_parameters->begin; a != node_type->type_parameters->end; a = a->next)
		{
			node_t *temp;
			temp = (node_t *)a->value;

			result = syntax_type_parameter(syntax, node, temp);
			if (!result)
			{
				return 0;
			}
		}
	}

	if (node_type->heritages)
	{
		for (a = node_type->heritages->begin; a != node_type->heritages->end; a = a->next)
		{
			node_t *temp;
			temp = (node_t *)a->value;

			result = syntax_heritage(syntax, node, temp);
			if (!result)
			{
				return 0;
			}
		}
	}

	result = syntax_expression(syntax, node, node_type->body);
	if (!result)
	{
		return 0;
	}

	return 1;
}

static void
syntax_check_export_object(syntax_t *syntax, node_t *node);

static void
syntax_check_is_export_object_key(syntax_t *syntax, node_t *node)
{
	if (node->kind == NODE_KIND_OBJECT)
	{
		syntax_check_export_object(syntax, node);
	}
	else if (node->kind != NODE_KIND_ID)
	{
		syntax_error(syntax, node->position,
									"The key of the export object must be of the identifier type or an object");
	}
}

static void
syntax_check_is_export_object_property(syntax_t *syntax, node_t *node)
{
	node_object_property_t *node_object_property;
	node_object_property = (node_object_property_t *)node->value;

	syntax_check_is_export_object_key(syntax, node_object_property->name);
}

static void
syntax_check_export_object(syntax_t *syntax, node_t *node)
{
	node_object_t *node_object;
	node_object = (node_object_t *)node->value;

	ilist_t *a;
	for (a = node_object->list->begin; a != node_object->list->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		syntax_check_is_export_object_property(syntax, temp);
	}
}

static int32_t
syntax_export(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_modifier_t *node_export;
	node_export = (node_modifier_t *)node->value;

	int32_t result = 0;
	switch (node_export->x->kind)
	{
	case NODE_KIND_CLASS:
		result = syntax_class(syntax, node, node_export->x);
		break;

	case NODE_KIND_ENUM:
		result = syntax_enum(syntax, node, node_export->x);
		break;

	case NODE_KIND_FUNC:
		result = syntax_func(syntax, node, node_export->x);
		break;

	case NODE_KIND_VAR:
		result = syntax_var(syntax, node, node_export->x);
		break;

	case NODE_KIND_CONST:
		result = syntax_const(syntax, node, node_export->x);
		break;

	case NODE_KIND_TYPE:
		result = syntax_type(syntax, node, node_export->x);
		break;

	case NODE_KIND_OBJECT:
		syntax_check_export_object(syntax, node_export->x);
		result = syntax_object(syntax, node, node_export->x);
		break;

	default:
		syntax_error(syntax, node->position, "unknown export declaration");
		return 0;
	}

	if (!result)
	{
		return 0;
	}

	return 1;
}

static int32_t
syntax_import(syntax_t *syntax, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_import_t *node_import;
	node_import = (node_import_t *)node->value;

	int32_t result = 0;
	result = syntax_string(syntax, node, node_import->path);
	if (!result)
	{
		return 0;
	}

	if(node_import->fields)
	{
		ilist_t *a;
		for (a = node_import->fields->begin; a != node_import->fields->end; a = a->next)
		{
			node_t *temp;
			temp = (node_t *)a->value;
			result = syntax_field(syntax, node, temp);
			if (!result)
			{
				return 0;
			}
		}
	}

	return 1;
}

static int32_t
syntax_module(syntax_t *syntax, node_t *node)
{
	if (!list_rpush(syntax->modules, (list_value_t)node))
	{
		return 0;
	}

	int32_t result;
	result = syntax_apply_import(syntax, node);
	if (!result)
	{
		return 0;
	}

	node->parent = NULL;

	node_module_t *node_module;
	node_module = (node_module_t *)node->value;

	ilist_t *a;
	for (a = node_module->members->begin; a != node_module->members->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		switch (temp->kind)
		{
		case NODE_KIND_EXPORT:
			result = syntax_export(syntax, node, temp);
			break;

		case NODE_KIND_IMPORT:
			result = syntax_import(syntax, node, temp);
			break;

		case NODE_KIND_CLASS:
			result = syntax_class(syntax, node, temp);
			break;

		case NODE_KIND_ENUM:
			result = syntax_enum(syntax, node, temp);
			break;

		case NODE_KIND_FUNC:
			result = syntax_func(syntax, node, temp);
			break;

		case NODE_KIND_VAR:
			result = syntax_var(syntax, node, temp);
			break;

		case NODE_KIND_CONST:
			result = syntax_const(syntax, node, temp);
			break;

		case NODE_KIND_TYPE:
			result = syntax_type(syntax, node, temp);
			break;

		default:
			syntax_error(syntax, node->position, "module: unknown type %ld", temp->kind);
			return 0;
		}
		if (!result)
		{
			return 0;
		}
	}
	return 1;
}

int32_t
syntax_run(syntax_t *syntax, node_t *node)
{
	int32_t result;
	result = syntax_module(syntax, node);
	if (!result || list_count(syntax->errors) > 0)
	{
		return 0;
	}
	/*
	ilist_t *a;
	for(a = syntax->modules->begin; a != syntax->modules->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		result = syntax_global_symbolization_module(syntax, temp);
		if(!result)
		{
			return 0;
		}
	}

	if (list_count(syntax->errors) > 0)
	{
		return 0;
	}
	*/

	return 1;
}

syntax_t *
syntax_create(program_t *program, list_t *errors)
{
	syntax_t *syntax;
	syntax = (syntax_t *)malloc(sizeof(syntax_t));
	if (!syntax)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(syntax_t));
		return NULL;
	}
	memset(syntax, 0, sizeof(syntax_t));

	syntax->program = program;
	syntax->errors = errors;

	syntax->modules = list_create();
	if (!syntax->modules)
	{
		return NULL;
	}
	return syntax;
}