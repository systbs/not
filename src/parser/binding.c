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
#include "forecast.h"
#include "error.h"
#include "binding.h"
#include "symbol.h"

static error_t *
binding_error(binding_t *binding, position_t position, const char *format, ...)
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

	if (list_rpush(binding->errors, (list_value_t)error))
	{
		return NULL;
	}

	return error;
}

static int32_t
binding_syntax_module(binding_t *binding, node_t *node);

static int32_t
binding_fetch_import_path(binding_t *binding, node_t *node, char *base_path)
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
			binding_error(binding, node->position, "root path is null, add 'QALAM-PATH' to environment");
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
					for (a = binding->modules->begin; a != binding->modules->end; a = a->next)
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
					parser = parser_create(binding->program, resolved_file, binding->errors);
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

					if (!binding_syntax_module(binding, module))
					{
						return 0;
					}
				}
			}
			closedir(dr);
		}
		else
		{
			binding_error(binding, node->position, "could not open dir '%s'", resolved_path);
			return 0;
		}
	}
	else
	{
		if (!path_exist(resolved_path))
		{
			if (is_absolute)
			{
				binding_error(binding, node->position, "module '%s' is not found", resolved_path);
				return 0;
			}
			binding_error(binding, node->position, "module '%s' is not in '%s'", resolved_path, directory_path);
			return 0;
		}

		int32_t exist_module = 0;
		ilist_t *a;
		for (a = binding->modules->begin; a != binding->modules->end; a = a->next)
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
		parser = parser_create(binding->program, resolved_path, binding->errors);
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

		if (!binding_syntax_module(binding, module))
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
binding_fetch_import(binding_t *binding, node_t *node, char *base_path)
{
	node_import_t *node_import;
	node_import = (node_import_t *)node->value;

	int32_t result;
	result = binding_fetch_import_path(binding, node_import->path, base_path);
	if (!result)
	{
		return 0;
	}

	return 1;
}

static int32_t
binding_apply_import(binding_t *binding, node_t *node)
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
			result = binding_fetch_import(binding, temp, node_module->path);
			if (!result)
			{
				return 0;
			}
		}
	}

	return 1;
}

static int32_t
binding_syntax_expression(binding_t *binding, node_t *parent, node_t *node);

static int32_t
binding_syntax_func(binding_t *binding, node_t *parent, node_t *node);

static int32_t
binding_syntax_parameter(binding_t *binding, node_t *parent, node_t *node);

static int32_t
binding_syntax_type_parameter(binding_t *binding, node_t *parent, node_t *node);

static int32_t
binding_syntax_postfix(binding_t *binding, node_t *parent, node_t *node);

static int32_t
binding_syntax_block(binding_t *binding, node_t *parent, node_t *node);

static int32_t
binding_syntax_const(binding_t *binding, node_t *parent, node_t *node);

static int32_t
binding_syntax_var(binding_t *binding, node_t *parent, node_t *node);

static int32_t
binding_syntax_type(binding_t *binding, node_t *parent, node_t *node);

static int32_t
binding_syntax_prefix(binding_t *binding, node_t *parent, node_t *node);

static int32_t
binding_syntax_export(binding_t *binding, node_t *parent, node_t *node);


static int32_t
binding_syntax_id(binding_t *binding, node_t *parent, node_t *node)
{
	node->parent = parent;
	return 1;
}

static int32_t
binding_syntax_number(binding_t *binding, node_t *parent, node_t *node)
{
	node->parent = parent;
	return 1;
}

static int32_t
binding_syntax_string(binding_t *binding, node_t *parent, node_t *node)
{
	node->parent = parent;
	return 1;
}

static int32_t
binding_syntax_char(binding_t *binding, node_t *parent, node_t *node)
{
	node->parent = parent;
	return 1;
}

static int32_t
binding_syntax_null(binding_t *binding, node_t *parent, node_t *node)
{
	node->parent = parent;
	return 1;
}

static int32_t
binding_syntax_true(binding_t *binding, node_t *parent, node_t *node)
{
	node->parent = parent;
	return 1;
}

static int32_t
binding_syntax_false(binding_t *binding, node_t *parent, node_t *node)
{
	node->parent = parent;
	return 1;
}

static int32_t
binding_syntax_array(binding_t *binding, node_t *parent, node_t *node)
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

		result = binding_syntax_expression(binding, node, temp);
		if (!result)
		{
			return 0;
		}
	}
	return 1;
}

static int32_t
binding_syntax_parenthesis(binding_t *binding, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_unary_t *node_unary;
	node_unary = (node_unary_t *)node->value;

	int32_t result;
	result = binding_syntax_expression(binding, node, node_unary->right);
	if (!result)
	{
		return 0;
	}

	return 1;
}

static int32_t
binding_syntax_object_property(binding_t *binding, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_object_property_t *node_object_property;
	node_object_property = (node_object_property_t *)node->value;

	int32_t result;
	result = binding_syntax_expression(binding, node, node_object_property->name);
	if (!result)
	{
		return 0;
	}

	if (node_object_property->value)
	{
		result = binding_syntax_expression(binding, node, node_object_property->value);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
binding_syntax_object(binding_t *binding, node_t *parent, node_t *node)
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

		result = binding_syntax_object_property(binding, node, temp);
		if (!result)
		{
			return 0;
		}
	}
	return 1;
}

static int32_t
binding_syntax_primary(binding_t *binding, node_t *parent, node_t *node)
{
	int32_t result;
	switch (node->kind)
	{
	case NODE_KIND_ID:
		result = binding_syntax_id(binding, parent, node);
		break;

	case NODE_KIND_NUMBER:
		result = binding_syntax_number(binding, parent, node);
		break;

	case NODE_KIND_STRING:
		result = binding_syntax_string(binding, parent, node);
		break;

	case NODE_KIND_CHAR:
		result = binding_syntax_char(binding, parent, node);
		break;

	case NODE_KIND_NULL:
		result = binding_syntax_null(binding, parent, node);
		break;

	case NODE_KIND_TRUE:
		result = binding_syntax_true(binding, parent, node);
		break;

	case NODE_KIND_FALSE:
		result = binding_syntax_false(binding, parent, node);
		break;

	case NODE_KIND_ARRAY:
		result = binding_syntax_array(binding, parent, node);
		break;

	case NODE_KIND_OBJECT:
		result = binding_syntax_object(binding, parent, node);
		break;

	case NODE_KIND_FUNC:
		result = binding_syntax_func(binding, parent, node);
		break;

	case NODE_KIND_PARENTHESIS:
		result = binding_syntax_parenthesis(binding, parent, node);
		break;

	default:
		binding_error(binding, node->position, "unknown primary node");
		return 0;
	}

	if (!result)
	{
		return 0;
	}

	return 1;
}

static void
binding_checking_cannot_followed_by_composite(binding_t *binding, node_t *node)
{
	node_composite_t *node_composite;
	node_composite = (node_composite_t *)node->value;

	if (node_composite->base->kind == NODE_KIND_GET_SLICE)
	{
		binding_error(binding, node->position,
									"A slice has no signatures for which the type argument list is applicable");
	}
}

static int32_t
binding_syntax_composite(binding_t *binding, node_t *parent, node_t *node)
{
	node->parent = parent;

	binding_checking_cannot_followed_by_composite(binding, node);

	node_composite_t *node_composite;
	node_composite = (node_composite_t *)node->value;

	int32_t result;
	result = binding_syntax_postfix(binding, node, node_composite->base);
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

			result = binding_syntax_expression(binding, node, temp);
			if (!result)
			{
				return 0;
			}
		}
	}

	return 1;
}

static void
binding_checking_cannot_followed_by_call(binding_t *binding, node_t *node)
{
	node_call_t *node_call;
	node_call = (node_call_t *)node->value;

	if (node_call->callable->kind == NODE_KIND_GET_SLICE)
	{
		binding_error(binding, node->position, "A slice cannot be called");
	}
}

static int32_t
binding_syntax_call(binding_t *binding, node_t *parent, node_t *node)
{
	node->parent = parent;

	binding_checking_cannot_followed_by_call(binding, node);

	node_call_t *node_call;
	node_call = (node_call_t *)node->value;

	int32_t result;
	result = binding_syntax_postfix(binding, node, node_call->callable);
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

			result = binding_syntax_expression(binding, node, temp);
			if (!result)
			{
				return 0;
			}
		}
	}

	return 1;
}

static void
binding_checking_cannot_followed_by_get_slice(binding_t *binding, node_t *node)
{
	node_get_slice_t *node_get_slice;
	node_get_slice = (node_get_slice_t *)node->value;

	if (node_get_slice->name->kind == NODE_KIND_COMPOSITE)
	{
		binding_error(binding, node->position, "An instantiation expression cannot be followed by a property access");
	}
}

static int32_t
binding_syntax_get_slice(binding_t *binding, node_t *parent, node_t *node)
{
	node->parent = parent;

	binding_checking_cannot_followed_by_get_slice(binding, node);

	node_get_slice_t *node_get_slice;
	node_get_slice = (node_get_slice_t *)node->value;

	int32_t result;
	result = binding_syntax_postfix(binding, node, node_get_slice->name);
	if (!result)
	{
		return 0;
	}

	if (node_get_slice->start)
	{
		result = binding_syntax_expression(binding, node, node_get_slice->start);
		if (!result)
		{
			return 0;
		}
	}

	if (node_get_slice->step)
	{
		result = binding_syntax_expression(binding, node, node_get_slice->step);
		if (!result)
		{
			return 0;
		}
	}

	if (node_get_slice->stop)
	{
		result = binding_syntax_expression(binding, node, node_get_slice->stop);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static void
binding_checking_cannot_followed_by_get_item(binding_t *binding, node_t *node)
{
	node_get_item_t *node_get_item;
	node_get_item = (node_get_item_t *)node->value;

	if (node_get_item->name->kind == NODE_KIND_COMPOSITE)
	{
		binding_error(binding, node->position, "An instantiation expression cannot be followed by a property access");
	}
}

static int32_t
binding_syntax_get_item(binding_t *binding, node_t *parent, node_t *node)
{
	node->parent = parent;

	binding_checking_cannot_followed_by_get_item(binding, node);

	node_get_item_t *node_get_item;
	node_get_item = (node_get_item_t *)node->value;

	int32_t result;
	result = binding_syntax_postfix(binding, node, node_get_item->name);
	if (!result)
	{
		return 0;
	}

	if (node_get_item->index)
	{
		result = binding_syntax_expression(binding, node, node_get_item->index);
		if (!result)
		{
			return 0;
		}
	}
	return 1;
}

static void
binding_checking_cannot_followed_by_get_attr(binding_t *binding, node_t *node)
{
	node_get_attr_t *node_get_attr;
	node_get_attr = (node_get_attr_t *)node->value;

	if (node_get_attr->left->kind == NODE_KIND_COMPOSITE)
	{
		binding_error(binding, node->position, "An instantiation expression cannot be followed by a property access");
	}

	if (node_get_attr->left->kind == NODE_KIND_GET_SLICE)
	{
		binding_error(binding, node->position, "A slice cannot be followed by a property access");
	}
}

static int32_t
binding_syntax_get_attr(binding_t *binding, node_t *parent, node_t *node)
{
	node->parent = parent;

	binding_checking_cannot_followed_by_get_attr(binding, node);

	node_get_attr_t *node_get_attr;
	node_get_attr = (node_get_attr_t *)node->value;

	int32_t result;
	result = binding_syntax_postfix(binding, node, node_get_attr->left);
	if (!result)
	{
		return 0;
	}

	result = binding_syntax_id(binding, node, node_get_attr->right);
	if (!result)
	{
		return 0;
	}
	return 1;
}

static int32_t
binding_syntax_postfix(binding_t *binding, node_t *parent, node_t *node)
{
	int32_t result;
	switch (node->kind)
	{
	case NODE_KIND_COMPOSITE:
		result = binding_syntax_composite(binding, parent, node);
		if (!result)
		{
			return 0;
		}
		break;

	case NODE_KIND_CALL:
		result = binding_syntax_call(binding, parent, node);
		if (!result)
		{
			return 0;
		}
		break;

	case NODE_KIND_GET_SLICE:
		result = binding_syntax_get_slice(binding, parent, node);
		if (!result)
		{
			return 0;
		}
		break;

	case NODE_KIND_GET_ITEM:
		result = binding_syntax_get_item(binding, parent, node);
		if (!result)
		{
			return 0;
		}
		break;

	case NODE_KIND_GET_ATTR:
		result = binding_syntax_get_attr(binding, parent, node);
		if (!result)
		{
			return 0;
		}
		break;

	default:
		result = binding_syntax_primary(binding, parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}
	return 1;
}

static int32_t
binding_syntax_prefix(binding_t *binding, node_t *parent, node_t *node)
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
			result = binding_syntax_prefix(binding, node, node_unary->right);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_unary->right->kind == NODE_KIND_NOT)
		{
			result = binding_syntax_prefix(binding, node, node_unary->right);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_unary->right->kind == NODE_KIND_POS)
		{
			result = binding_syntax_prefix(binding, node, node_unary->right);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_unary->right->kind == NODE_KIND_NEG)
		{
			result = binding_syntax_prefix(binding, node, node_unary->right);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_unary->right->kind == NODE_KIND_GET_VALUE)
		{
			result = binding_syntax_prefix(binding, node, node_unary->right);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_unary->right->kind == NODE_KIND_GET_ADDRESS)
		{
			result = binding_syntax_prefix(binding, node, node_unary->right);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_unary->right->kind == NODE_KIND_AWAIT)
		{
			result = binding_syntax_prefix(binding, node, node_unary->right);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_unary->right->kind == NODE_KIND_SIZEOF)
		{
			result = binding_syntax_prefix(binding, node, node_unary->right);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_unary->right->kind == NODE_KIND_TYPEOF)
		{
			result = binding_syntax_prefix(binding, node, node_unary->right);
			if (!result)
			{
				return 0;
			}
		}
		else
		{
			result = binding_syntax_postfix(binding, node, node_unary->right);
			if (!result)
			{
				return 0;
			}
		}
		break;

	default:
		result = binding_syntax_postfix(binding, parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}

	return 1;
}

static int32_t
binding_syntax_multiplicative(binding_t *binding, node_t *parent, node_t *node)
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
			result = binding_syntax_multiplicative(binding, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_binary->left->kind == NODE_KIND_DIV)
		{
			result = binding_syntax_multiplicative(binding, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_binary->left->kind == NODE_KIND_MOD)
		{
			result = binding_syntax_multiplicative(binding, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else
		{
			result = binding_syntax_prefix(binding, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}

		result = binding_syntax_prefix(binding, node, node_binary->right);
		if (!result)
		{
			return 0;
		}
		break;

	default:
		result = binding_syntax_prefix(binding, parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}

	return 1;
}

static int32_t
binding_syntax_addative(binding_t *binding, node_t *parent, node_t *node)
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
			result = binding_syntax_addative(binding, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_binary->left->kind == NODE_KIND_MINUS)
		{
			result = binding_syntax_addative(binding, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else
		{
			result = binding_syntax_multiplicative(binding, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}

		result = binding_syntax_multiplicative(binding, node, node_binary->right);
		if (!result)
		{
			return 0;
		}
		break;

	default:
		result = binding_syntax_multiplicative(binding, parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}

	return 1;
}

static int32_t
binding_syntax_shifting(binding_t *binding, node_t *parent, node_t *node)
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
			result = binding_syntax_shifting(binding, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_binary->left->kind == NODE_KIND_SHR)
		{
			result = binding_syntax_shifting(binding, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else
		{
			result = binding_syntax_addative(binding, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}

		result = binding_syntax_addative(binding, node, node_binary->right);
		if (!result)
		{
			return 0;
		}
		break;

	default:
		result = binding_syntax_addative(binding, parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}

	return 1;
}

static int32_t
binding_syntax_relational(binding_t *binding, node_t *parent, node_t *node)
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
			result = binding_syntax_relational(binding, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_binary->left->kind == NODE_KIND_LE)
		{
			result = binding_syntax_relational(binding, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_binary->left->kind == NODE_KIND_GT)
		{
			result = binding_syntax_relational(binding, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_binary->left->kind == NODE_KIND_GE)
		{
			result = binding_syntax_relational(binding, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else
		{
			result = binding_syntax_shifting(binding, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}

		result = binding_syntax_shifting(binding, node, node_binary->right);
		if (!result)
		{
			return 0;
		}
		break;

	default:
		result = binding_syntax_shifting(binding, parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}

	return 1;
}

static int32_t
binding_syntax_equality(binding_t *binding, node_t *parent, node_t *node)
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
			result = binding_syntax_equality(binding, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_binary->left->kind == NODE_KIND_EQ)
		{
			result = binding_syntax_equality(binding, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else if (node_binary->left->kind == NODE_KIND_NEQ)
		{
			result = binding_syntax_equality(binding, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else
		{
			result = binding_syntax_relational(binding, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}

		result = binding_syntax_relational(binding, node, node_binary->right);
		if (!result)
		{
			return 0;
		}
		break;

	default:
		result = binding_syntax_relational(binding, parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}

	return 1;
}

static int32_t
binding_syntax_bitwise_and(binding_t *binding, node_t *parent, node_t *node)
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
			result = binding_syntax_bitwise_and(binding, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else
		{
			result = binding_syntax_equality(binding, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}

		result = binding_syntax_equality(binding, node, node_binary->right);
		if (!result)
		{
			return 0;
		}
		break;

	default:
		result = binding_syntax_equality(binding, parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}

	return 1;
}

static int32_t
binding_syntax_bitwise_xor(binding_t *binding, node_t *parent, node_t *node)
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
			result = binding_syntax_bitwise_xor(binding, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else
		{
			result = binding_syntax_bitwise_and(binding, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}

		result = binding_syntax_bitwise_and(binding, node, node_binary->right);
		if (!result)
		{
			return 0;
		}
		break;

	default:
		result = binding_syntax_bitwise_and(binding, parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}

	return 1;
}

static int32_t
binding_syntax_bitwise_or(binding_t *binding, node_t *parent, node_t *node)
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
			result = binding_syntax_bitwise_or(binding, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else
		{
			result = binding_syntax_bitwise_xor(binding, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}

		result = binding_syntax_bitwise_xor(binding, node, node_binary->right);
		if (!result)
		{
			return 0;
		}
		break;

	default:
		result = binding_syntax_bitwise_xor(binding, parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}

	return 1;
}

static int32_t
binding_syntax_logical_and(binding_t *binding, node_t *parent, node_t *node)
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
			result = binding_syntax_logical_and(binding, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else
		{
			result = binding_syntax_bitwise_or(binding, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}

		result = binding_syntax_bitwise_or(binding, node, node_binary->right);
		if (!result)
		{
			return 0;
		}
		break;

	default:
		result = binding_syntax_bitwise_or(binding, parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}

	return 1;
}

static int32_t
binding_syntax_logical_or(binding_t *binding, node_t *parent, node_t *node)
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
			result = binding_syntax_logical_or(binding, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}
		else
		{
			result = binding_syntax_logical_and(binding, node, node_binary->left);
			if (!result)
			{
				return 0;
			}
		}

		result = binding_syntax_logical_and(binding, node, node_binary->right);
		if (!result)
		{
			return 0;
		}
		break;

	default:
		result = binding_syntax_logical_and(binding, parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}

	return 1;
}

static int32_t
binding_syntax_conditional(binding_t *binding, node_t *parent, node_t *node)
{
	int32_t result;
	node_conditional_t *node_conditional;
	switch (node->kind)
	{
	case NODE_KIND_CONDITIONAL:
		node->parent = parent;
		node_conditional = (node_conditional_t *)node->value;
		result = binding_syntax_logical_or(binding, node, node_conditional->condition);
		if (!result)
		{
			return 0;
		}
		result = binding_syntax_conditional(binding, node, node_conditional->true_expression);
		if (!result)
		{
			return 0;
		}
		result = binding_syntax_conditional(binding, node, node_conditional->false_expression);
		if (!result)
		{
			return 0;
		}
		break;

	default:
		result = binding_syntax_logical_or(binding, parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}

	return 1;
}

static int32_t
binding_syntax_expression(binding_t *binding, node_t *parent, node_t *node)
{
	return binding_syntax_conditional(binding, parent, node);
}

static int32_t
binding_syntax_assign(binding_t *binding, node_t *parent, node_t *node)
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
		result = binding_syntax_expression(binding, node, node_binary->left);
		if (!result)
		{
			return 0;
		}

		result = binding_syntax_expression(binding, node, node_binary->right);
		if (!result)
		{
			return 0;
		}
		break;

	default:
		result = binding_syntax_expression(binding, parent, node);
		if (!result)
		{
			return 0;
		}
		break;
	}

	return 1;
}

static int32_t
binding_syntax_if(binding_t *binding, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_if_t *node_if;
	node_if = (node_if_t *)node->value;

	int32_t result;
	result = binding_syntax_expression(binding, node, node_if->condition);
	if (!result)
	{
		return 0;
	}

	result = binding_syntax_block(binding, node, node_if->then_body);
	if (!result)
	{
		return 0;
	}

	if (node_if->else_body)
	{
		switch (node_if->else_body->kind)
		{
		case NODE_KIND_IF:
			result = binding_syntax_if(binding, node, node_if->else_body);
			break;

		default:
			result = binding_syntax_block(binding, node, node_if->else_body);
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
binding_syntax_catch(binding_t *binding, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_catch_t *node_catch;
	node_catch = (node_catch_t *)node->value;

	int32_t result;
	if (node_catch->parameter)
	{
		result = binding_syntax_parameter(binding, node, node_catch->parameter);
		if (!result)
		{
			return 0;
		}
	}

	result = binding_syntax_block(binding, node, node_catch->body);
	if (!result)
	{
		return 0;
	}

	return 1;
}

static int32_t
binding_syntax_try(binding_t *binding, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_try_t *node_try;
	node_try = (node_try_t *)node->value;

	int32_t result;
	result = binding_syntax_block(binding, node, node_try->body);
	if (!result)
	{
		return 0;
	}

	ilist_t *a;
	for (a = node_try->catchs->begin; a != node_try->catchs->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;
		result = binding_syntax_catch(binding, node, temp);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
binding_syntax_for_init(binding_t *binding, node_t *parent, node_t *node)
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
			result = binding_syntax_var(binding, node, temp);
			break;

		case NODE_KIND_CONST:
			result = binding_syntax_const(binding, node, temp);
			break;

		default:
			result = binding_syntax_assign(binding, node, temp);
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
binding_syntax_for_step(binding_t *binding, node_t *parent, node_t *node)
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

		result = binding_syntax_assign(binding, node, temp);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
binding_syntax_for(binding_t *binding, node_t *parent, node_t *node)
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
			result = binding_syntax_for_init(binding, node, node_for->initializer);
			break;

		case NODE_KIND_VAR:
			result = binding_syntax_var(binding, node, node_for->initializer);
			break;

		case NODE_KIND_CONST:
			result = binding_syntax_const(binding, node, node_for->initializer);
			break;

		default:
			result = binding_syntax_assign(binding, node, node_for->initializer);
			break;
		}
	}

	if (node_for->condition)
	{
		result = binding_syntax_expression(binding, node, node_for->condition);
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
			result = binding_syntax_for_step(binding, node, node_for->incrementor);
			break;

		default:
			result = binding_syntax_assign(binding, node, node_for->incrementor);
			break;
		}

		if (!result)
		{
			return 0;
		}
	}

	result = binding_syntax_block(binding, node, node_for->body);
	if (!result)
	{
		return 0;
	}

	return 1;
}

static int32_t
binding_syntax_forin(binding_t *binding, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_forin_t *node_forin;
	node_forin = (node_forin_t *)node->value;

	int32_t result;
	switch (node_forin->initializer->kind)
	{
	case NODE_KIND_VAR:
		result = binding_syntax_var(binding, node, node_forin->initializer);
		break;

	case NODE_KIND_CONST:
		result = binding_syntax_const(binding, node, node_forin->initializer);
		break;

	default:
		binding_error(binding, node->position, "unknown initializer for..in loop");
		return 0;
	}
	if (!result)
	{
		return 0;
	}

	result = binding_syntax_expression(binding, node, node_forin->expression);
	if (!result)
	{
		return 0;
	}

	result = binding_syntax_block(binding, node, node_forin->body);
	if (!result)
	{
		return 0;
	}

	return 1;
}

static int32_t
binding_syntax_break(binding_t *binding, node_t *parent, node_t *node)
{
	node->parent = parent;
	return 1;
}

static int32_t
binding_syntax_continue(binding_t *binding, node_t *parent, node_t *node)
{
	node->parent = parent;
	return 1;
}

static int32_t
binding_syntax_return(binding_t *binding, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_return_t *node_return;
	node_return = (node_return_t *)node->value;

	int32_t result = binding_syntax_expression(binding, node, node_return->expression);
	if (!result)
	{
		return 0;
	}

	return 1;
}

static int32_t
binding_syntax_throw(binding_t *binding, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_throw_t *node_throw;
	node_throw = (node_throw_t *)node->value;

	int32_t result = binding_syntax_expression(binding, node, node_throw->expression);
	if (!result)
	{
		return 0;
	}

	return 1;
}

static int32_t
binding_syntax_statement(binding_t *binding, node_t *parent, node_t *node)
{
	int32_t result = 0;
	switch (node->kind)
	{
	case NODE_KIND_BLOCK:
		result = binding_syntax_block(binding, parent, node);
		break;

	case NODE_KIND_IF:
		result = binding_syntax_if(binding, parent, node);
		break;

	case NODE_KIND_TRY:
		result = binding_syntax_try(binding, parent, node);
		break;

	case NODE_KIND_FOR:
		result = binding_syntax_for(binding, parent, node);
		break;

	case NODE_KIND_FORIN:
		result = binding_syntax_forin(binding, parent, node);
		break;

	case NODE_KIND_VAR:
		result = binding_syntax_var(binding, parent, node);
		break;

	case NODE_KIND_CONST:
		result = binding_syntax_const(binding, parent, node);
		break;

	case NODE_KIND_TYPE:
		result = binding_syntax_type(binding, parent, node);
		break;

	case NODE_KIND_BREAK:
		result = binding_syntax_break(binding, parent, node);
		break;

	case NODE_KIND_CONTINUE:
		result = binding_syntax_continue(binding, parent, node);
		break;

	case NODE_KIND_RETURN:
		result = binding_syntax_return(binding, parent, node);
		break;

	case NODE_KIND_THROW:
		result = binding_syntax_throw(binding, parent, node);
		break;

	case NODE_KIND_FUNC:
		result = binding_syntax_func(binding, parent, node);
		break;

	default:
		result = binding_syntax_assign(binding, parent, node);
		break;
	}

	if (!result)
	{
		return 0;
	}

	return 1;
}

static int32_t
binding_syntax_block(binding_t *binding, node_t *parent, node_t *node)
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

		result = binding_syntax_statement(binding, node, temp);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static void
binding_checking_is_type(binding_t *binding, node_t *node);

static int32_t
binding_syntax_type_parameter(binding_t *binding, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_type_parameter_t *node_type_parameter;
	node_type_parameter = (node_type_parameter_t *)node->value;

	int32_t result;
	result = binding_syntax_id(binding, node, node_type_parameter->name);
	if (!result)
	{
		return 0;
	}

	if (node_type_parameter->extends)
	{
		binding_checking_is_type(binding, node_type_parameter->extends);

		result = binding_syntax_expression(binding, node, node_type_parameter->extends);
		if (!result)
		{
			return 0;
		}
	}

	if (node_type_parameter->value)
	{
		binding_checking_is_type(binding, node_type_parameter->value);

		result = binding_syntax_expression(binding, node, node_type_parameter->value);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
binding_syntax_enum_member(binding_t *binding, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_enum_member_t *node_enum_member;
	node_enum_member = (node_enum_member_t *)node->value;

	int32_t result;
	result = binding_syntax_id(binding, node, node_enum_member->name);
	if (!result)
	{
		return 0;
	}

	if (node_enum_member->value)
	{
		result = binding_syntax_expression(binding, node, node_enum_member->value);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
binding_syntax_enum(binding_t *binding, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_enum_t *node_enum;
	node_enum = (node_enum_t *)node->value;

	int32_t result;
	result = binding_syntax_id(binding, node, node_enum->name);
	if (!result)
	{
		return 0;
	}

	ilist_t *a;
	for (a = node_enum->body->begin; a != node_enum->body->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		result = binding_syntax_enum_member(binding, node, temp);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
binding_syntax_property(binding_t *binding, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_field_t *node_field;
	node_field = (node_field_t *)node->value;

	int32_t result;
	result = binding_syntax_id(binding, node, node_field->name);
	if (!result)
	{
		return 0;
	}

	if (node_field->type)
	{
		result = binding_syntax_expression(binding, node, node_field->type);
		if (!result)
		{
			return 0;
		}
	}

	if (node_field->value)
	{
		result = binding_syntax_expression(binding, node, node_field->value);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static void
binding_checking_is_object_variable(binding_t *binding, node_t *node);

static void
binding_checking_is_get_attr(binding_t *binding, node_t *node)
{
	if (node->kind == NODE_KIND_GET_ATTR)
	{
		node_get_attr_t *node_get_attr;
		node_get_attr = (node_get_attr_t *)node->value;

		binding_checking_is_get_attr(binding, node_get_attr->left);
		binding_checking_is_get_attr(binding, node_get_attr->right);
	}
	else if (node->kind != NODE_KIND_ID)
	{
		binding_error(binding, node->position, "expected identifire");
	}
}

static void
binding_checking_is_object_variable_value(binding_t *binding, node_t *node)
{
	if (node->kind == NODE_KIND_OBJECT)
	{
		binding_checking_is_object_variable(binding, node);
	}
	else if (node->kind == NODE_KIND_GET_ATTR)
	{
		binding_checking_is_get_attr(binding, node);
	}
	else if (node->kind != NODE_KIND_ID)
	{
		binding_error(binding, node->position, "expected rhs variable");
	}
}

static void
binding_checking_is_object_variable_name(binding_t *binding, node_t *node)
{
	if (node->kind == NODE_KIND_OBJECT)
	{
		binding_checking_is_object_variable(binding, node);
	}
	else if (node->kind != NODE_KIND_ID)
	{
		binding_error(binding, node->position, "expected variable name");
	}
}

static void
binding_checking_is_object_property_variable(binding_t *binding, node_t *node)
{
	node_object_property_t *node_object_property;
	node_object_property = (node_object_property_t *)node->value;

	if (node_object_property->name)
	{
		binding_checking_is_object_variable_name(binding, node_object_property->name);
	}

	if (node_object_property->value)
	{
		binding_checking_is_object_variable_value(binding, node_object_property->value);
	}
}

static void
binding_checking_is_object_variable(binding_t *binding, node_t *node)
{
	node_object_t *node_object;
	node_object = (node_object_t *)node->value;

	ilist_t *a;
	for (a = node_object->list->begin; a != node_object->list->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;
		binding_checking_is_object_property_variable(binding, temp);
	}
}

static void
binding_checking_is_variable_name(binding_t *binding, node_t *node)
{
	if (node->kind != NODE_KIND_ID)
	{
		binding_error(binding, node->position, "expected variable name");
	}
}

static void
binding_checking_is_array_variable(binding_t *binding, node_t *node)
{
	node_array_t *node_array;
	node_array = (node_array_t *)node->value;

	ilist_t *a;
	for (a = node_array->list->begin; a != node_array->list->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		binding_checking_is_variable_name(binding, temp);
	}
}

static void
binding_checking_is_ellipsisiable(binding_t *binding, node_t *node)
{
	if (node->kind == NODE_KIND_ARRAY)
	{
		binding_checking_is_array_variable(binding, node);
	}
	else if (node->kind != NODE_KIND_ID)
	{
		binding_error(binding, node->position, "expected identifier or array variable");
	}
}

static void
binding_checking_is_parameter_ellipsis(binding_t *binding, node_t *node)
{
	node_unary_t *node_unary;
	node_unary = (node_unary_t *)node->value;

	binding_checking_is_ellipsisiable(binding, node_unary->right);
}

static void
binding_checking_is_parameter_name(binding_t *binding, node_t *node)
{
	if (node->kind == NODE_KIND_OBJECT)
	{
		binding_checking_is_object_variable(binding, node);
	}
	else if (node->kind == NODE_KIND_ARRAY)
	{
		binding_checking_is_array_variable(binding, node);
	}
	else if (node->kind == NODE_KIND_ELLIPSIS)
	{
		binding_checking_is_parameter_ellipsis(binding, node);
	}
	else if (node->kind != NODE_KIND_ID)
	{
		binding_error(binding, node->position, "expected parameter");
	}
}

static void
binding_checking_is_type_array(binding_t *binding, node_t *node)
{
	node_array_t *node_array;
	node_array = (node_array_t *)node->value;

	ilist_t *a;
	for (a = node_array->list->begin; a != node_array->list->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		binding_checking_is_type(binding, temp);
	}
}

static void
binding_checking_is_type_object_key(binding_t *binding, node_t *node)
{
	if (
			(node->kind != NODE_KIND_ID) &&
			(node->kind != NODE_KIND_STRING) &&
			(node->kind != NODE_KIND_NUMBER) &&
			(node->kind != NODE_KIND_CHAR))
	{
		binding_error(binding, node->position, "expected identifier");
	}
}

static void
binding_checking_is_type_object_property(binding_t *binding, node_t *node)
{
	node_object_property_t *node_object_property;
	node_object_property = (node_object_property_t *)node->value;
	binding_checking_is_type_object_key(binding, node_object_property->name);
	if (node_object_property->value)
	{
		binding_checking_is_type(binding, node_object_property->value);
	}
}

static void
binding_checking_is_type_object(binding_t *binding, node_t *node)
{
	node_object_t *node_object;
	node_object = (node_object_t *)node->value;

	ilist_t *a;
	for (a = node_object->list->begin; a != node_object->list->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		binding_checking_is_type_object_property(binding, temp);
	}
}

static void
binding_checking_is_type_func(binding_t *binding, node_t *node)
{
	node_func_t *node_func;
	node_func = (node_func_t *)node->value;
	if (node_func->fields)
	{
		binding_error(binding, node->position, "no fields in func type");
	}
	if (node_func->name)
	{
		binding_error(binding, node->position, "no name in func type");
	}
	if (node_func->body)
	{
		binding_error(binding, node->position, "no body in func type");
	}
}

static void
binding_checking_is_type_parenthesis(binding_t *binding, node_t *node)
{
	node_unary_t *node_unary;
	node_unary = (node_unary_t *)node->value;

	binding_checking_is_type(binding, node_unary->right);
}

static void
binding_checking_is_type_primary(binding_t *binding, node_t *node)
{
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
		binding_checking_is_type_array(binding, node);
		break;

	case NODE_KIND_OBJECT:
		binding_checking_is_type_object(binding, node);
		break;

	case NODE_KIND_FUNC:
		binding_checking_is_type_func(binding, node);
		break;

	case NODE_KIND_PARENTHESIS:
		binding_checking_is_type_parenthesis(binding, node);
		break;

	default:
		binding_error(binding, node->position, "expected type");
		break;
	}
}

static void
binding_checking_is_type_composite(binding_t *binding, node_t *node)
{
	node_composite_t *node_composite;
	node_composite = (node_composite_t *)node->value;

	if (node_composite->type_arguments)
	{
		ilist_t *a;
		for (a = node_composite->type_arguments->begin; a != node_composite->type_arguments->end; a = a->next)
		{
			node_t *temp;
			temp = (node_t *)a->value;

			binding_checking_is_type(binding, temp);
		}
	}
}

static void
binding_checking_is_type_postfix(binding_t *binding, node_t *node)
{
	switch (node->kind)
	{
	case NODE_KIND_COMPOSITE:
		binding_checking_is_type_composite(binding, node);
		break;

	case NODE_KIND_CALL:
	case NODE_KIND_GET_SLICE:
		binding_error(binding, node->position, "expected type");
		break;

	case NODE_KIND_GET_ITEM:
	case NODE_KIND_GET_ATTR:
		break;

	default:
		binding_checking_is_type_primary(binding, node);
		break;
	}
}

static void
binding_checking_is_type_unary(binding_t *binding, node_t *node)
{
	node_unary_t *node_unary;
	switch (node->kind)
	{
	case NODE_KIND_TYPEOF:
	case NODE_KIND_GET_VALUE:
	case NODE_KIND_GET_ADDRESS:
		node_unary = (node_unary_t *)node->value;
		binding_checking_is_type_unary(binding, node_unary->right);
		break;

	case NODE_KIND_TILDE:
	case NODE_KIND_NOT:
	case NODE_KIND_NEG:
	case NODE_KIND_POS:
	case NODE_KIND_SIZEOF:
	case NODE_KIND_ELLIPSIS:
		binding_error(binding, node->position, "expected type");
		break;

	default:
		binding_checking_is_type_postfix(binding, node);
		break;
	}
}

static void
binding_checking_is_type_binary(binding_t *binding, node_t *node)
{
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
		binding_error(binding, node->position, "expected type operator");
		break;

	case NODE_KIND_OR:
	case NODE_KIND_AND:
	case NODE_KIND_NEQ:
	case NODE_KIND_EQ:
	case NODE_KIND_EXTENDS:
		node_binary = (node_binary_t *)node->value;
		binding_checking_is_type_binary(binding, node_binary->left);
		binding_checking_is_type_binary(binding, node_binary->right);
		break;

	default:
		binding_checking_is_type_unary(binding, node);
		break;
	}
}

static void
binding_checking_is_type_conditional(binding_t *binding, node_t *node)
{
	if (node->kind == NODE_KIND_CONDITIONAL)
	{
		node_conditional_t *node_conditional;
		node_conditional = (node_conditional_t *)node->value;

		binding_checking_is_type_binary(binding, node_conditional->condition);
		binding_checking_is_type_conditional(binding, node_conditional->true_expression);
		binding_checking_is_type_conditional(binding, node_conditional->false_expression);
	}
	else
	{
		binding_checking_is_type_binary(binding, node);
	}
}

static void
binding_checking_is_type(binding_t *binding, node_t *node)
{
	binding_checking_is_type_conditional(binding, node);
}

static int32_t
binding_syntax_parameter(binding_t *binding, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_parameter_t *node_parameter;
	node_parameter = (node_parameter_t *)node->value;

	binding_checking_is_parameter_name(binding, node_parameter->name);

	int32_t result;
	result = binding_syntax_expression(binding, node, node_parameter->name);
	if (!result)
	{
		return 0;
	}

	if (node_parameter->type)
	{
		binding_checking_is_type(binding, node_parameter->type);

		result = binding_syntax_expression(binding, node, node_parameter->type);
		if (!result)
		{
			return 0;
		}
	}

	if (node_parameter->value)
	{
		binding_checking_is_type(binding, node_parameter->value);

		result = binding_syntax_expression(binding, node, node_parameter->value);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
binding_syntax_field(binding_t *binding, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_field_t *node_field;
	node_field = (node_field_t *)node->value;

	int32_t result;
	result = binding_syntax_id(binding, node, node_field->name);
	if (!result)
	{
		return 0;
	}

	binding_checking_is_type(binding, node_field->type);

	result = binding_syntax_expression(binding, node, node_field->type);
	if (!result)
	{
		return 0;
	}

	return 1;
}

static int32_t
binding_syntax_method(binding_t *binding, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_method_t *node_method;
	node_method = (node_method_t *)node->value;

	int32_t result;
	result = binding_syntax_id(binding, node, node_method->name);
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

			result = binding_syntax_parameter(binding, node, temp);
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

			result = binding_syntax_type_parameter(binding, node, temp);
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
			result = binding_syntax_block(binding, node, node_method->body);
		}
		else
		{
			result = binding_syntax_expression(binding, node, node_method->body);
		}
	}
	
	if (!result)
	{
		return 0;
	}

	return 1;
}

static int32_t
binding_syntax_class(binding_t *binding, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_class_t *node_class;
	node_class = (node_class_t *)node->value;

	int32_t result;
	result = binding_syntax_id(binding, node, node_class->name);
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

			result = binding_syntax_expression(binding, node, temp);
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

			result = binding_syntax_type_parameter(binding, node, temp);
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
		case NODE_KIND_CLASS:
			result = binding_syntax_class(binding, node, temp);
			if (!result)
			{
				return 0;
			}
			break;

		case NODE_KIND_ENUM:
			result = binding_syntax_enum(binding, node, temp);
			if (!result)
			{
				return 0;
			}
			break;

		case NODE_KIND_PROPERTY:
			result = binding_syntax_property(binding, node, temp);
			if (!result)
			{
				return 0;
			}
			break;

		case NODE_KIND_METHOD:
			result = binding_syntax_method(binding, node, temp);
			if (!result)
			{
				return 0;
			}
			break;

		default:
			binding_error(binding, node->position, "unknown node body decalaration");
			return 0;
		}
	}

	return 1;
}

static int32_t
binding_syntax_func(binding_t *binding, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_func_t *node_func;
	node_func = (node_func_t *)node->value;

	int32_t result;
	if (node_func->name)
	{
		result = binding_syntax_id(binding, node, node_func->name);
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

			result = binding_syntax_parameter(binding, node, temp);
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

			result = binding_syntax_type_parameter(binding, node, temp);
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
			result = binding_syntax_field(binding, node, temp);
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
			result = binding_syntax_block(binding, node, node_func->body);
		}
		else
		{
			result = binding_syntax_expression(binding, node, node_func->body);
		}
		
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
binding_checking_node_is_name(binding_t *binding, node_t *node)
{
	if (node->kind != NODE_KIND_ID)
	{
		return 0;
	}
	return 1;
}

static int32_t
binding_checking_node_is_hierarchy_of_name(binding_t *binding, node_t *node)
{
	int32_t result;
	if (node->kind == NODE_KIND_GET_ATTR)
	{
		node_get_attr_t *node_get_attr;
		node_get_attr = (node_get_attr_t *)node->value;
		result = binding_checking_node_is_hierarchy_of_name(binding, node_get_attr->left);
		if (!result)
		{
			return 0;
		}
		result = binding_checking_node_is_name(binding, node_get_attr->right);
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

static void
binding_checking_object_property_is_reilable_var_object_property(binding_t *binding, node_t *node)
{
	node_object_property_t *node_object_property;
	node_object_property = (node_object_property_t *)node->value;

	int32_t result;
	result = binding_checking_node_is_name(binding, node_object_property->name);
	if (!result)
	{
		if (node_object_property->value)
		{
			binding_error(binding, node_object_property->name->position,
										"The name must be an identifier");
		}
		else
		{
			binding_error(binding, node_object_property->name->position,
										"Valuation must be with field name of object or nested object");
		}
	}
	if (node_object_property->value)
	{
		result = binding_checking_node_is_hierarchy_of_name(binding, node_object_property->value);
		if (!result)
		{
			binding_error(binding, node_object_property->value->position,
										"Valuation must be with identifier of object or nested object");
		}
	}
}

static void
binding_checking_object_is_reilable_var_object(binding_t *binding, node_t *node)
{
	node_object_t *node_object;
	node_object = (node_object_t *)node->value;

	ilist_t *a;
	for (a = node_object->list->begin; a != node_object->list->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		binding_checking_object_property_is_reilable_var_object_property(binding, temp);
	}
}

static void
binding_checking_array_is_reilable_var_array(binding_t *binding, node_t *node)
{
	node_array_t *node_array;
	node_array = (node_array_t *)node->value;

	ilist_t *a;
	for (a = node_array->list->begin; a != node_array->list->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		int32_t result;
		result = binding_checking_node_is_name(binding, temp);
		if (!result)
		{
			binding_error(binding, temp->position,
										"In valuing by array, the fields must include the variable name");
		}
	}
}

static int32_t
binding_syntax_var(binding_t *binding, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_var_t *node_var;
	node_var = (node_var_t *)node->value;

	int32_t result;
	if (node_var->name->kind == NODE_KIND_OBJECT)
	{
		binding_checking_object_is_reilable_var_object(binding, node_var->name);
		result = binding_syntax_object(binding, node, node_var->name);
		if (!result)
		{
			return 0;
		}
	}
	else if (node_var->name->kind == NODE_KIND_ARRAY)
	{
		binding_checking_array_is_reilable_var_array(binding, node_var->name);
		result = binding_syntax_array(binding, node, node_var->name);
		if (!result)
		{
			return 0;
		}
	}
	else
	{
		result = binding_syntax_id(binding, node, node_var->name);
		if (!result)
		{
			return 0;
		}
	}

	if (node_var->type)
	{
		result = binding_syntax_expression(binding, node, node_var->type);
		if (!result)
		{
			return 0;
		}
	}
	if (node_var->value)
	{
		result = binding_syntax_expression(binding, node, node_var->value);
		if (!result)
		{
			return 0;
		}
	}
	return 1;
}

static int32_t
binding_syntax_const(binding_t *binding, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_const_t *node_const;
	node_const = (node_const_t *)node->value;

	int32_t result;
	if (node_const->name->kind == NODE_KIND_OBJECT)
	{
		binding_checking_object_is_reilable_var_object(binding, node_const->name);
		result = binding_syntax_object(binding, node, node_const->name);
		if (!result)
		{
			return 0;
		}
	}
	else if (node_const->name->kind == NODE_KIND_ARRAY)
	{
		binding_checking_array_is_reilable_var_array(binding, node_const->name);
		result = binding_syntax_array(binding, node, node_const->name);
		if (!result)
		{
			return 0;
		}
	}
	else
	{
		result = binding_syntax_id(binding, node, node_const->name);
		if (!result)
		{
			return 0;
		}
	}

	if (node_const->type)
	{
		result = binding_syntax_expression(binding, node, node_const->type);
		if (!result)
		{
			return 0;
		}
	}
	if (node_const->value)
	{
		result = binding_syntax_expression(binding, node, node_const->value);
		if (!result)
		{
			return 0;
		}
	}
	return 1;
}

static int32_t
binding_syntax_type(binding_t *binding, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_type_t *node_type;
	node_type = (node_type_t *)node->value;

	int32_t result;
	result = binding_syntax_id(binding, node, node_type->name);
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

			result = binding_syntax_type_parameter(binding, node, temp);
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

			result = binding_syntax_expression(binding, node, temp);
			if (!result)
			{
				return 0;
			}
		}
	}

	result = binding_syntax_expression(binding, node, node_type->body);
	if (!result)
	{
		return 0;
	}

	return 1;
}

static int32_t
binding_syntax_namespace(binding_t *binding, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_namespace_t *node_namespace;
	node_namespace = (node_namespace_t *)node->value;

	int32_t result = 0;
	result = binding_syntax_id(binding, node, node_namespace->name);
	if (!result)
	{
		return 0;
	}

	ilist_t *a;
	for (a = node_namespace->body->begin; a != node_namespace->body->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		switch (temp->kind)
		{
		case NODE_KIND_EXPORT:
			result = binding_syntax_export(binding, node, temp);
			break;

		case NODE_KIND_NAMESPACE:
			result = binding_syntax_namespace(binding, node, temp);
			break;

		case NODE_KIND_CLASS:
			result = binding_syntax_class(binding, node, temp);
			break;

		case NODE_KIND_ENUM:
			result = binding_syntax_enum(binding, node, temp);
			break;

		case NODE_KIND_FUNC:
			result = binding_syntax_func(binding, node, temp);
			break;

		case NODE_KIND_VAR:
			result = binding_syntax_var(binding, node, temp);
			break;

		case NODE_KIND_CONST:
			result = binding_syntax_const(binding, node, temp);
			break;

		case NODE_KIND_TYPE:
			result = binding_syntax_type(binding, node, temp);
			break;

		default:
			binding_error(binding, node->position, "unknown namespace member");
			return 0;
		}

		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static void
binding_checking_object_is_reilable_export_object(binding_t *binding, node_t *node);

static void
binding_checking_object_property_key_is_export_definable(binding_t *binding, node_t *node)
{
	if (node->kind == NODE_KIND_OBJECT)
	{
		binding_checking_object_is_reilable_export_object(binding, node);
	}
	else if (node->kind != NODE_KIND_ID)
	{
		binding_error(binding, node->position,
									"The key of the export object must be of the identifier type or an object");
	}
}

static void
binding_checking_object_property_is_reilable_export_object_property(binding_t *binding, node_t *node)
{
	node_object_property_t *node_object_property;
	node_object_property = (node_object_property_t *)node->value;

	binding_checking_object_property_key_is_export_definable(binding, node_object_property->name);
}

static void
binding_checking_object_is_reilable_export_object(binding_t *binding, node_t *node)
{
	node_object_t *node_object;
	node_object = (node_object_t *)node->value;

	ilist_t *a;
	for (a = node_object->list->begin; a != node_object->list->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		binding_checking_object_property_is_reilable_export_object_property(binding, temp);
	}
}

static int32_t
binding_syntax_export(binding_t *binding, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_modifier_t *node_export;
	node_export = (node_modifier_t *)node->value;

	int32_t result = 0;
	switch (node_export->x->kind)
	{
	case NODE_KIND_NAMESPACE:
		result = binding_syntax_namespace(binding, node, node_export->x);
		break;

	case NODE_KIND_CLASS:
		result = binding_syntax_class(binding, node, node_export->x);
		break;

	case NODE_KIND_ENUM:
		result = binding_syntax_enum(binding, node, node_export->x);
		break;

	case NODE_KIND_FUNC:
		result = binding_syntax_func(binding, node, node_export->x);
		break;

	case NODE_KIND_VAR:
		result = binding_syntax_var(binding, node, node_export->x);
		break;

	case NODE_KIND_CONST:
		result = binding_syntax_const(binding, node, node_export->x);
		break;

	case NODE_KIND_TYPE:
		result = binding_syntax_type(binding, node, node_export->x);
		break;

	case NODE_KIND_OBJECT:
		binding_checking_object_is_reilable_export_object(binding, node_export->x);
		result = binding_syntax_object(binding, node, node_export->x);
		break;

	default:
		binding_error(binding, node->position, "unknown export declaration");
		return 0;
	}

	if (!result)
	{
		return 0;
	}

	return 1;
}

static int32_t
binding_syntax_import(binding_t *binding, node_t *parent, node_t *node)
{
	node->parent = parent;

	node_import_t *node_import;
	node_import = (node_import_t *)node->value;

	int32_t result = 0;
	result = binding_syntax_string(binding, node, node_import->path);
	if (!result)
	{
		return 0;
	}

	ilist_t *a;
	for (a = node_import->fields->begin; a != node_import->fields->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		result = binding_syntax_field(binding, node, temp);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
binding_syntax_module(binding_t *binding, node_t *node)
{
	if (!list_rpush(binding->modules, (list_value_t)node))
	{
		return 0;
	}

	int32_t result;
	result = binding_apply_import(binding, node);
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
		case NODE_KIND_NAMESPACE:
			result = binding_syntax_namespace(binding, node, temp);
			break;

		case NODE_KIND_EXPORT:
			result = binding_syntax_export(binding, node, temp);
			break;

		case NODE_KIND_IMPORT:
			result = binding_syntax_import(binding, node, temp);
			break;

		case NODE_KIND_CLASS:
			result = binding_syntax_class(binding, node, temp);
			break;

		case NODE_KIND_ENUM:
			result = binding_syntax_enum(binding, node, temp);
			break;

		case NODE_KIND_FUNC:
			result = binding_syntax_func(binding, node, temp);
			break;

		case NODE_KIND_VAR:
			result = binding_syntax_var(binding, node, temp);
			break;

		case NODE_KIND_CONST:
			result = binding_syntax_const(binding, node, temp);
			break;

		case NODE_KIND_TYPE:
			result = binding_syntax_type(binding, node, temp);
			break;

		default:
			binding_error(binding, node->position, "unknown module member");
			return 0;
		}
		if (!result)
		{
			return 0;
		}
	}
	return 1;
}





static symbol_t *
binding_create_symbol_from_id(binding_t *binding, node_t *node, uint64_t flags, node_t *declaration)
{
	node_basic_t *node_basic;
	node_basic = (node_basic_t *)node->value;

	return symbol_create(node_basic->value, flags, declaration);
}

static node_t *
binding_get_container(binding_t *binding, node_t *node)
{
	node_t *parent;
	parent = node->parent;

	while(parent)
	{
		switch(parent->kind)
		{
		case NODE_KIND_OBJECT:
		case NODE_KIND_TYPE:
		case NODE_KIND_FOR:
		case NODE_KIND_FORIN:
		case NODE_KIND_IF:
		case NODE_KIND_TRY:
		case NODE_KIND_FUNC:
		case NODE_KIND_CLASS:
		case NODE_KIND_METHOD:
		case NODE_KIND_ENUM:
		case NODE_KIND_MODULE:
			return parent;

		case NODE_KIND_BLOCK:
			if((parent->parent->kind == NODE_KIND_FUNC) || (parent->parent->kind == NODE_KIND_METHOD))
			{
				return parent->parent;
			}
			return parent;
		
		default:
			parent = parent->parent;
			break;
		}
	}

	return NULL;
}

static node_t *
binding_get_scoped_container(binding_t *binding, node_t *node)
{
	node_t *parent;
	parent = node->parent;

	while(parent)
	{
		switch(parent->kind)
		{
		case NODE_KIND_TYPE:
		case NODE_KIND_FOR:
		case NODE_KIND_FORIN:
		case NODE_KIND_IF:
		case NODE_KIND_TRY:
		case NODE_KIND_FUNC:
		case NODE_KIND_CLASS:
		case NODE_KIND_METHOD:
		case NODE_KIND_ENUM:
		case NODE_KIND_MODULE:
			return parent;

		case NODE_KIND_BLOCK:
			if((parent->parent->kind == NODE_KIND_FUNC) || (parent->parent->kind == NODE_KIND_METHOD))
			{
				return parent->parent;
			}
			return parent;
		
		default:
			parent = parent->parent;
			break;
		}
	}

	return NULL;
}

static symbol_t *
binding_check_is_duplicated_symbol(binding_t *binding, node_t *container, symbol_t *symbol)
{

	while(container)
	{
		ilist_t *a;
		for(a = container->locals->begin; a != container->locals->end; a = a->next)
		{
			symbol_t *symbol2;
			symbol2 = (symbol_t *)a->value;

			if(strcmp(symbol->escaped_name, symbol2->escaped_name) == 0)
			{
				if(symbol_check_flag(symbol, SYMBOL_FLAG_FUNCTION))
				{
					
				}

				if(symbol_check_flag(symbol2, SYMBOL_FLAG_FUNCTION))
				{

				}
				return symbol2;
			}
		}

		container = binding_get_container(binding, container);
	}

	return NULL;
}

static int32_t 
binding_global_symbolization_parameter_by_object_and_array(binding_t *binding, node_t *node);

static int32_t 
binding_global_symbolization_parameter_object_property(binding_t *binding, node_t *node)
{
	node_object_property_t *node_object_property;
	node_object_property = (node_object_property_t *)node->value;

	int32_t result;
	result = binding_global_symbolization_parameter_by_object_and_array(binding, node_object_property->name);
	if (!result)
	{
		return 0;
	}

	return 1;
}

static int32_t 
binding_global_symbolization_parameter_object(binding_t *binding, node_t *node)
{
	node_object_t *node_object;
	node_object = (node_object_t *)node->value;

	int32_t result;
	ilist_t *a;
	for (a = node_object->list->begin; a != node_object->list->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		result = binding_global_symbolization_parameter_object_property(binding, temp);
		if (!result)
		{
			return 0;
		}
	}
	return 1;
}

static int32_t 
binding_global_symbolization_parameter_array(binding_t *binding, node_t *node)
{
	node_array_t *node_array;
	node_array = (node_array_t *)node->value;

	int32_t result;
	ilist_t *a;
	for (a = node_array->list->begin; a != node_array->list->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		result = binding_global_symbolization_parameter_by_object_and_array(binding, temp);
		if (!result)
		{
			return 0;
		}
	}
	return 1;
}

static int32_t 
binding_global_symbolization_parameter_ellipsis(binding_t *binding, node_t *node)
{
	node_unary_t *node_unary;
	node_unary = (node_unary_t *)node->value;

	int32_t result;
	result = binding_global_symbolization_parameter_by_object_and_array(binding, node_unary->right);
	if (!result)
	{
		return 0;
	}

	return 1;
}

static int32_t 
binding_global_symbolization_parameter_by_object_and_array(binding_t *binding, node_t *node)
{
	if(node->kind == NODE_KIND_OBJECT)
	{
		binding_global_symbolization_parameter_object(binding, node);
	}
	else if(node->kind == NODE_KIND_ARRAY)
	{
		binding_global_symbolization_parameter_array(binding, node);
	}
	else if(node->kind == NODE_KIND_ELLIPSIS)
	{
		binding_global_symbolization_parameter_ellipsis(binding, node);
	}
	else if(node->kind == NODE_KIND_ID)
	{
		symbol_t *symbol;
		symbol = binding_create_symbol_from_id(binding, node, SYMBOL_FLAG_PARAMETER, node);
		if(symbol)
		{
			node_t *container;
			container = binding_get_scoped_container(binding, node);
			if(container)
			{
				symbol_t *previous_symbol;
				if((previous_symbol = binding_check_is_duplicated_symbol(binding, container, symbol)))
				{
					binding_error(binding, node->position, "duplicate definition of '%s'", symbol->escaped_name);
				}
				else
				{
					if(!list_rpush(container->locals, (list_value_t)symbol))
					{
						return 0;
					}
				}
			}
			else
			{
				binding_error(binding, node->position, "empty container");
				return 0;
			}
		}
		else
		{
			binding_error(binding, node->position, "empty symbol");
			return 0;
		}
	}
	else 
	{
		binding_error(binding, node->position, "expected parameter name");
		return 0;
	}

	return 1;
}

static int32_t
binding_global_symbolization_parameter(binding_t *binding, node_t *node)
{
	node_parameter_t *node_parameter;
	node_parameter = (node_parameter_t *)node->value;

	int32_t result;
	result = binding_global_symbolization_parameter_by_object_and_array(binding, node_parameter->name);
	if (!result)
	{
		return 0;
	}

	return 1;
}

static int32_t
binding_global_symbolization_type_parameter(binding_t *binding, node_t *node)
{
	node_type_parameter_t *node_type_parameter;
	node_type_parameter = (node_type_parameter_t *)node->value;

	symbol_t *symbol;
	symbol = binding_create_symbol_from_id(binding, node_type_parameter->name, SYMBOL_FLAG_TYPE_PARAMETER, node);
	if(symbol)
	{
		node_t *container;
		container = binding_get_container(binding, node);
		if(container)
		{
			symbol_t *previous_symbol;
			if((previous_symbol = binding_check_is_duplicated_symbol(binding, container, symbol)))
			{
				binding_error(binding, node->position, "duplicate definition of '%s'", symbol->escaped_name);
			}
			else
			{
				if(!list_rpush(container->locals, (list_value_t)symbol))
				{
					return 0;
				}
			}
		}
		else
		{
			binding_error(binding, node->position, "empty container");
			return 0;
		}
	}
	else
	{
		binding_error(binding, node_type_parameter->name->position, "empty symbol");
		return 0;
	}

	return 1;
}

static int32_t
binding_global_symbolization_field(binding_t *binding, node_t *node)
{
	node_field_t *node_field;
	node_field = (node_field_t *)node->value;

	symbol_t *symbol;
	symbol = binding_create_symbol_from_id(binding, node_field->name, SYMBOL_FLAG_FIELD, node);
	if(symbol)
	{
		node_t *container;
		container = binding_get_container(binding, node);
		if(container)
		{
			symbol_t *previous_symbol;
			if((previous_symbol = binding_check_is_duplicated_symbol(binding, container, symbol)))
			{
				binding_error(binding, node->position, "duplicate definition of '%s'", symbol->escaped_name);
			}
			else
			{
				if(!list_rpush(container->locals, (list_value_t)symbol))
				{
					return 0;
				}
			}
		}
		else
		{
			binding_error(binding, node->position, "empty container");
			return 0;
		}
	}
	else
	{
		binding_error(binding, node_field->name->position, "empty symbol");
		return 0;
	}

	return 1;
}

static int32_t
binding_global_symbolization_method(binding_t *binding, node_t *node)
{
	node_method_t *node_method;
	node_method = (node_method_t *)node->value;

	symbol_t *symbol;
	symbol = binding_create_symbol_from_id(binding, node_method->name, SYMBOL_FLAG_METHOD, node);
	if(symbol)
	{
		node_t *container;
		container = binding_get_container(binding, node);
		if(container)
		{
			symbol_t *previous_symbol;
			if((previous_symbol = binding_check_is_duplicated_symbol(binding, container, symbol)))
			{
				binding_error(binding, node->position, "duplicate definition of '%s'", symbol->escaped_name);
			}
			else
			{
				if(!list_rpush(container->locals, (list_value_t)symbol))
				{
					return 0;
				}
			}
		}
		else
		{
			binding_error(binding, node->position, "empty container");
			return 0;
		}
	}
	else
	{
		binding_error(binding, node_method->name->position, "empty symbol");
		return 0;
	}

	int32_t result;

	ilist_t *a;
	if (node_method->type_parameters)
	{
		for (a = node_method->type_parameters->begin; a != node_method->type_parameters->end; a = a->next)
		{
			node_t *temp;
			temp = (node_t *)a->value;

			result = binding_global_symbolization_type_parameter(binding, temp);
			if (!result)
			{
				return 0;
			}
		}
	}

	if (node_method->parameters)
	{
		for (a = node_method->parameters->begin; a != node_method->parameters->end; a = a->next)
		{
			node_t *temp;
			temp = (node_t *)a->value;

			result = binding_global_symbolization_parameter(binding, temp);
			if (!result)
			{
				return 0;
			}
		}
	}

	return 1;
}

static int32_t
binding_global_symbolization_func(binding_t *binding, node_t *node)
{
	node_func_t *node_func;
	node_func = (node_func_t *)node->value;

	int32_t result;
	if (node_func->name)
	{
		symbol_t *symbol;
		symbol = binding_create_symbol_from_id(binding, node_func->name, SYMBOL_FLAG_METHOD, node);
		if(symbol)
		{
			node_t *container;
			container = binding_get_container(binding, node);
			if(container)
			{
				symbol_t *previous_symbol;
				if((previous_symbol = binding_check_is_duplicated_symbol(binding, container, symbol)))
				{
					binding_error(binding, node->position, "duplicate definition of '%s'", symbol->escaped_name);
				}
				else
				{
					if(!list_rpush(container->locals, (list_value_t)symbol))
					{
						return 0;
					}
				}
			}
			else
			{
				binding_error(binding, node->position, "empty container");
				return 0;
			}
		}
		else
		{
			binding_error(binding, node_func->name->position, "empty symbol");
			return 0;
		}
	}

	if (node_func->fields)
	{
		ilist_t *a;
		for (a = node_func->fields->begin; a != node_func->fields->end; a = a->next)
		{
			node_t *temp;
			temp = (node_t *)a->value;
			result = binding_global_symbolization_field(binding, temp);
			if (!result)
			{
				return 0;
			}
		}
	}

	if (node_func->type_parameters)
	{
		ilist_t *a;
		for (a = node_func->type_parameters->begin; a != node_func->type_parameters->end; a = a->next)
		{
			node_t *temp;
			temp = (node_t *)a->value;

			result = binding_global_symbolization_type_parameter(binding, temp);
			if (!result)
			{
				return 0;
			}
		}
	}

	if (node_func->parameters)
	{
		ilist_t *a;
		for (a = node_func->parameters->begin; a != node_func->parameters->end; a = a->next)
		{
			node_t *temp;
			temp = (node_t *)a->value;

			result = binding_global_symbolization_parameter(binding, temp);
			if (!result)
			{
				return 0;
			}
		}
	}

	return 1;
}

static int32_t
binding_global_symbolization_property(binding_t *binding, node_t *node)
{
	node_field_t *node_field;
	node_field = (node_field_t *)node->value;

	symbol_t *symbol;
	symbol = binding_create_symbol_from_id(binding, node_field->name, SYMBOL_FLAG_PROPERTY, node);
	if(symbol)
	{
		node_t *container;
		container = binding_get_container(binding, node);
		if(container)
		{
			symbol_t *previous_symbol;
			if((previous_symbol = binding_check_is_duplicated_symbol(binding, container, symbol)))
			{
				binding_error(binding, node->position, "duplicate definition of '%s'", symbol->escaped_name);
			}
			else
			{
				if(!list_rpush(container->locals, (list_value_t)symbol))
				{
					return 0;
				}
			}
		}
		else
		{
			binding_error(binding, node->position, "empty container");
			return 0;
		}
	}
	else
	{
		binding_error(binding, node_field->name->position, "empty symbol");
		return 0;
	}

	return 1;
}

static int32_t
binding_global_symbolization_enum_member(binding_t *binding, node_t *node)
{
	node_enum_member_t *node_enum_member;
	node_enum_member = (node_enum_member_t *)node->value;

	symbol_t *symbol;
	symbol = binding_create_symbol_from_id(binding, node_enum_member->name, SYMBOL_FLAG_ENUM_MEMBER, node);
	if(symbol)
	{
		node_t *container;
		container = binding_get_container(binding, node);
		if(container)
		{
			symbol_t *previous_symbol;
			if((previous_symbol = binding_check_is_duplicated_symbol(binding, container, symbol)))
			{
				binding_error(binding, node->position, "duplicate definition of '%s'", symbol->escaped_name);
			}
			else
			{
				if(!list_rpush(container->locals, (list_value_t)symbol))
				{
					return 0;
				}
			}
		}
		else
		{
			binding_error(binding, node->position, "empty container");
			return 0;
		}
	}
	else
	{
		binding_error(binding, node_enum_member->name->position, "empty symbol");
		return 0;
	}

	return 1;
}

static int32_t
binding_global_symbolization_enum(binding_t *binding, node_t *node)
{
	node_enum_t *node_enum;
	node_enum = (node_enum_t *)node->value;

	symbol_t *symbol;
	symbol = binding_create_symbol_from_id(binding, node_enum->name, SYMBOL_FLAG_ENUM, node);
	if(symbol)
	{
		node_t *container;
		container = binding_get_container(binding, node);
		if(container)
		{
			symbol_t *previous_symbol;
			if((previous_symbol = binding_check_is_duplicated_symbol(binding, container, symbol)))
			{
				binding_error(binding, node->position, "duplicate definition of '%s'", symbol->escaped_name);
			}
			else
			{
				if(!list_rpush(container->locals, (list_value_t)symbol))
				{
					return 0;
				}
			}
		}
		else
		{
			binding_error(binding, node->position, "empty container");
			return 0;
		}
	}
	else
	{
		binding_error(binding, node_enum->name->position, "empty symbol");
		return 0;
	}

	int32_t result;

	ilist_t *a;
	for (a = node_enum->body->begin; a != node_enum->body->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		result = binding_global_symbolization_enum_member(binding, temp);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
binding_global_symbolization_class(binding_t *binding, node_t *node)
{
	node_class_t *node_class;
	node_class = (node_class_t *)node->value;

	symbol_t *symbol;
	symbol = binding_create_symbol_from_id(binding, node_class->name, SYMBOL_FLAG_CLASS, node);
	if(symbol)
	{
		node_t *container;
		container = binding_get_container(binding, node);
		if(container)
		{
			symbol_t *previous_symbol;
			if((previous_symbol = binding_check_is_duplicated_symbol(binding, container, symbol)))
			{
				binding_error(binding, node->position, "duplicate definition of '%s'", symbol->escaped_name);
			}
			else
			{
				if(!list_rpush(container->locals, (list_value_t)symbol))
				{
					return 0;
				}
			}
		}
		else
		{
			binding_error(binding, node->position, "empty container");
			return 0;
		}
	}
	else
	{
		binding_error(binding, node_class->name->position, "empty symbol");
		return 0;
	}

	int32_t result;

	ilist_t *a;
	if (node_class->type_parameters)
	{
		for (a = node_class->type_parameters->begin; a != node_class->type_parameters->end; a = a->next)
		{
			node_t *temp;
			temp = (node_t *)a->value;

			result = binding_global_symbolization_type_parameter(binding, temp);
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
		case NODE_KIND_CLASS:
			result = binding_global_symbolization_class(binding, temp);
			if (!result)
			{
				return 0;
			}
			break;

		case NODE_KIND_ENUM:
			result = binding_global_symbolization_enum(binding, temp);
			if (!result)
			{
				return 0;
			}
			break;

		case NODE_KIND_PROPERTY:
			result = binding_global_symbolization_property(binding, temp);
			if (!result)
			{
				return 0;
			}
			break;

		case NODE_KIND_METHOD:
			result = binding_global_symbolization_method(binding, temp);
			if (!result)
			{
				return 0;
			}
			break;

		default:
			binding_error(binding, node->position, "unknown node body decalaration");
			return 0;
		}
	}

	return 1;
}


static int32_t 
binding_global_symbolization_variable_by_object_and_array(binding_t *binding, node_t *node);

static int32_t 
binding_global_symbolization_variable_object_property(binding_t *binding, node_t *node)
{
	node_object_property_t *node_object_property;
	node_object_property = (node_object_property_t *)node->value;

	int32_t result;
	result = binding_global_symbolization_variable_by_object_and_array(binding, node_object_property->name);
	if (!result)
	{
		return 0;
	}

	return 1;
}

static int32_t 
binding_global_symbolization_variable_object(binding_t *binding, node_t *node)
{
	node_object_t *node_object;
	node_object = (node_object_t *)node->value;

	int32_t result;
	ilist_t *a;
	for (a = node_object->list->begin; a != node_object->list->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		result = binding_global_symbolization_variable_object_property(binding, temp);
		if (!result)
		{
			return 0;
		}
	}
	return 1;
}

static int32_t 
binding_global_symbolization_variable_array(binding_t *binding, node_t *node)
{
	node_array_t *node_array;
	node_array = (node_array_t *)node->value;

	int32_t result;
	ilist_t *a;
	for (a = node_array->list->begin; a != node_array->list->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		result = binding_global_symbolization_variable_by_object_and_array(binding, temp);
		if (!result)
		{
			return 0;
		}
	}
	return 1;
}

static int32_t 
binding_global_symbolization_variable_ellipsis(binding_t *binding, node_t *node)
{
	node_unary_t *node_unary;
	node_unary = (node_unary_t *)node->value;

	int32_t result;
	result = binding_global_symbolization_variable_by_object_and_array(binding, node_unary->right);
	if (!result)
	{
		return 0;
	}

	return 1;
}

static int32_t 
binding_global_symbolization_variable_by_object_and_array(binding_t *binding, node_t *node)
{
	if(node->kind == NODE_KIND_OBJECT)
	{
		binding_global_symbolization_variable_object(binding, node);
	}
	else if(node->kind == NODE_KIND_ARRAY)
	{
		binding_global_symbolization_variable_array(binding, node);
	}
	else if(node->kind == NODE_KIND_ELLIPSIS)
	{
		binding_global_symbolization_variable_ellipsis(binding, node);
	}
	else if(node->kind == NODE_KIND_ID)
	{
		symbol_t *symbol;
		symbol = binding_create_symbol_from_id(binding, node, SYMBOL_FLAG_VARIABLE, node);
		if(symbol)
		{
			node_t *container;
			container = binding_get_scoped_container(binding, node);
			if(container)
			{
				symbol_t *previous_symbol;
				if((previous_symbol = binding_check_is_duplicated_symbol(binding, container, symbol)))
				{
					binding_error(binding, node->position, "duplicate definition of '%s'", symbol->escaped_name);
				}
				else
				{
					if(!list_rpush(container->locals, (list_value_t)symbol))
					{
						return 0;
					}
				}
			}
			else
			{
				binding_error(binding, node->position, "empty container");
				return 0;
			}
		}
		else
		{
			binding_error(binding, node->position, "empty symbol");
			return 0;
		}
	}
	else 
	{
		binding_error(binding, node->position, "expected parameter name");
		return 0;
	}

	return 1;
}

static int32_t
binding_global_symbolization_var(binding_t *binding, node_t *node)
{
	node_var_t *node_var;
	node_var = (node_var_t *)node->value;

	int32_t result;
	result = binding_global_symbolization_variable_by_object_and_array(binding, node_var->name);
	if (!result)
	{
		return 0;
	}

	return 1;
}

static int32_t 
binding_global_symbolization_constant_by_object_and_array(binding_t *binding, node_t *node);

static int32_t 
binding_global_symbolization_constant_object_property(binding_t *binding, node_t *node)
{
	node_object_property_t *node_object_property;
	node_object_property = (node_object_property_t *)node->value;

	int32_t result;
	result = binding_global_symbolization_constant_by_object_and_array(binding, node_object_property->name);
	if (!result)
	{
		return 0;
	}

	return 1;
}

static int32_t 
binding_global_symbolization_constant_object(binding_t *binding, node_t *node)
{
	node_object_t *node_object;
	node_object = (node_object_t *)node->value;

	int32_t result;
	ilist_t *a;
	for (a = node_object->list->begin; a != node_object->list->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		result = binding_global_symbolization_constant_object_property(binding, temp);
		if (!result)
		{
			return 0;
		}
	}
	return 1;
}

static int32_t 
binding_global_symbolization_constant_array(binding_t *binding, node_t *node)
{
	node_array_t *node_array;
	node_array = (node_array_t *)node->value;

	int32_t result;
	ilist_t *a;
	for (a = node_array->list->begin; a != node_array->list->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		result = binding_global_symbolization_constant_by_object_and_array(binding, temp);
		if (!result)
		{
			return 0;
		}
	}
	return 1;
}

static int32_t 
binding_global_symbolization_constant_ellipsis(binding_t *binding, node_t *node)
{
	node_unary_t *node_unary;
	node_unary = (node_unary_t *)node->value;

	int32_t result;
	result = binding_global_symbolization_constant_by_object_and_array(binding, node_unary->right);
	if (!result)
	{
		return 0;
	}

	return 1;
}

static int32_t 
binding_global_symbolization_constant_by_object_and_array(binding_t *binding, node_t *node)
{
	if(node->kind == NODE_KIND_OBJECT)
	{
		binding_global_symbolization_constant_object(binding, node);
	}
	else if(node->kind == NODE_KIND_ARRAY)
	{
		binding_global_symbolization_constant_array(binding, node);
	}
	else if(node->kind == NODE_KIND_ELLIPSIS)
	{
		binding_global_symbolization_constant_ellipsis(binding, node);
	}
	else if(node->kind == NODE_KIND_ID)
	{
		symbol_t *symbol;
		symbol = binding_create_symbol_from_id(binding, node, SYMBOL_FLAG_CONST, node);
		if(symbol)
		{
			node_t *container;
			container = binding_get_scoped_container(binding, node);
			if(container)
			{
				symbol_t *previous_symbol;
				if((previous_symbol = binding_check_is_duplicated_symbol(binding, container, symbol)))
				{
					binding_error(binding, node->position, "duplicate definition of '%s'", symbol->escaped_name);
				}
				else
				{
					if(!list_rpush(container->locals, (list_value_t)symbol))
					{
						return 0;
					}
				}
			}
			else
			{
				binding_error(binding, node->position, "empty container");
				return 0;
			}
		}
		else
		{
			binding_error(binding, node->position, "empty symbol");
			return 0;
		}
	}
	else 
	{
		binding_error(binding, node->position, "expected parameter name");
		return 0;
	}

	return 1;
}

static int32_t
binding_global_symbolization_const(binding_t *binding, node_t *node)
{
	node_const_t *node_const;
	node_const = (node_const_t *)node->value;

	int32_t result;
	result = binding_global_symbolization_constant_by_object_and_array(binding, node_const->name);
	if (!result)
	{
		return 0;
	}

	return 1;
}

static int32_t
binding_global_symbolization_type(binding_t *binding, node_t *node)
{
	node_type_t *node_type;
	node_type = (node_type_t *)node->value;

	symbol_t *symbol;
	symbol = binding_create_symbol_from_id(binding, node_type->name, SYMBOL_FLAG_TYPE, node);
	if(symbol)
	{
		node_t *container;
		container = binding_get_container(binding, node);
		if(container)
		{
			symbol_t *previous_symbol;
			if((previous_symbol = binding_check_is_duplicated_symbol(binding, container, symbol)))
			{
				binding_error(binding, node->position, "duplicate definition of '%s'", symbol->escaped_name);
			}
			else
			{
				if(!list_rpush(container->locals, (list_value_t)symbol))
				{
					return 0;
				}
			}
		}
		else
		{
			binding_error(binding, node->position, "empty container");
			return 0;
		}
	}
	else
	{
		binding_error(binding, node_type->name->position, "empty symbol");
		return 0;
	}

	int32_t result;

	ilist_t *a;
	if (node_type->type_parameters)
	{
		for (a = node_type->type_parameters->begin; a != node_type->type_parameters->end; a = a->next)
		{
			node_t *temp;
			temp = (node_t *)a->value;

			result = binding_global_symbolization_type_parameter(binding, temp);
			if (!result)
			{
				return 0;
			}
		}
	}

	return 1;
}

static int32_t
binding_global_symbolization_export_by_object(binding_t *binding, node_t *node);

static int32_t 
binding_global_symbolization_export_object_property(binding_t *binding, node_t *node)
{
	node_object_property_t *node_object_property;
	node_object_property = (node_object_property_t *)node->value;

	int32_t result;
	result = binding_global_symbolization_export_by_object(binding, node_object_property->name);
	if (!result)
	{
		return 0;
	}

	return 1;
}

static int32_t
binding_global_symbolization_export_object(binding_t *binding, node_t *node)
{
	node_object_t *node_object;
	node_object = (node_object_t *)node->value;

	int32_t result;
	ilist_t *a;
	for (a = node_object->list->begin; a != node_object->list->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		result = binding_global_symbolization_export_object_property(binding, temp);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
binding_global_symbolization_export_by_object(binding_t *binding, node_t *node)
{
	if(node->kind == NODE_KIND_OBJECT)
	{
		int32_t result;
		result = binding_global_symbolization_export_object(binding, node);
		if (!result)
		{
			return 0;
		}
	}
	else if(node->kind == NODE_KIND_ID)
	{
		symbol_t *symbol;
		symbol = binding_create_symbol_from_id(binding, node, SYMBOL_FLAG_EXPORT, node);
		if(symbol)
		{
			node_t *container;
			container = binding_get_container(binding, node);
			if(container)
			{
				symbol_t *previous_symbol;
				if((previous_symbol = binding_check_is_duplicated_symbol(binding, container, symbol)))
				{
					binding_error(binding, node->position, "duplicate definition of '%s'", symbol->escaped_name);
				}
				else
				{
					if(!list_rpush(container->locals, (list_value_t)symbol))
					{
						return 0;
					}
				}
			}
			else
			{
				binding_error(binding, node->position, "empty container");
				return 0;
			}
		}
		else
		{
			binding_error(binding, node->position, "empty symbol");
			return 0;
		}
	}

	return 1;
}

static int32_t
binding_global_symbolization_export(binding_t *binding, node_t *node)
{
	node_modifier_t *node_export;
	node_export = (node_modifier_t *)node->value;

	int32_t result = 0;
	switch (node_export->x->kind)
	{
	case NODE_KIND_CLASS:
		result = binding_global_symbolization_class(binding, node_export->x);
		break;

	case NODE_KIND_ENUM:
		result = binding_global_symbolization_enum(binding, node_export->x);
		break;

	case NODE_KIND_FUNC:
		result = binding_global_symbolization_func(binding, node_export->x);
		break;

	case NODE_KIND_VAR:
		result = binding_global_symbolization_var(binding, node_export->x);
		break;

	case NODE_KIND_CONST:
		result = binding_global_symbolization_const(binding, node_export->x);
		break;

	case NODE_KIND_TYPE:
		result = binding_global_symbolization_type(binding, node_export->x);
		break;

	case NODE_KIND_OBJECT:
		result = binding_global_symbolization_export_by_object(binding, node_export->x);
		break;

	default:
		binding_error(binding, node->position, "unknown export declaration");
		return 0;
	}

	if (!result)
	{
		return 0;
	}

	return 1;
}

static int32_t
binding_global_symbolization_import(binding_t *binding, node_t *node)
{
	node_import_t *node_import;
	node_import = (node_import_t *)node->value;

	int32_t result;
	ilist_t *a;
	for (a = node_import->fields->begin; a != node_import->fields->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		result = binding_global_symbolization_field(binding, temp);
		if (!result)
		{
			return 0;
		}
	}

	return 1;
}

static int32_t
binding_global_symbolization_module(binding_t *binding, node_t *node)
{
	node_module_t *node_module;
	node_module = (node_module_t *)node->value;

	int32_t result = 0;

	ilist_t *a;
	for (a = node_module->members->begin; a != node_module->members->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;
		switch (temp->kind)
		{
		case NODE_KIND_EXPORT:
			result = binding_global_symbolization_export(binding, temp);
			break;

		case NODE_KIND_IMPORT:
			result = binding_global_symbolization_import(binding, temp);
			break;

		case NODE_KIND_CLASS:
			result = binding_global_symbolization_class(binding, temp);
			break;

		case NODE_KIND_ENUM:
			result = binding_global_symbolization_enum(binding, temp);
			break;

		case NODE_KIND_FUNC:
			result = binding_global_symbolization_func(binding, temp);
			break;

		case NODE_KIND_VAR:
			result = binding_global_symbolization_var(binding, temp);
			break;

		case NODE_KIND_CONST:
			result = binding_global_symbolization_const(binding, temp);
			break;

		case NODE_KIND_TYPE:
			result = binding_global_symbolization_type(binding, temp);
			break;

		default:
			binding_error(binding, node->position, "unknown module member");
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
binding_run(binding_t *binding, node_t *node)
{
	int32_t result;
	result = binding_syntax_module(binding, node);
	if (!result || list_count(binding->errors) > 0)
	{
		return 0;
	}

	ilist_t *a;
	for(a = binding->modules->begin; a != binding->modules->end; a = a->next)
	{
		node_t *temp;
		temp = (node_t *)a->value;

		result = binding_global_symbolization_module(binding, temp);
		if(!result)
		{
			return 0;
		}
	}

	if (list_count(binding->errors) > 0)
	{
		return 0;
	}

	return 1;
}

binding_t *
binding_create(program_t *program, list_t *errors)
{
	binding_t *binding;
	binding = (binding_t *)malloc(sizeof(binding_t));
	if (!binding)
	{
		fprintf(stderr, "unable to allocted a block of %zu bytes\n", sizeof(binding_t));
		return NULL;
	}
	memset(binding, 0, sizeof(binding_t));

	binding->program = program;
	binding->errors = errors;

	binding->modules = list_create();
	if (!binding->modules)
	{
		return NULL;
	}
	return binding;
}