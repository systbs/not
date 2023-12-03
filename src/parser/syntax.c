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
#include "symbol.h"
#include "graph.h"
#include "syntax.h"

static error_t *
syntax_error(graph_t *graph, symbol_t *current, const char *format, ...)
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

	node_t *node;
	node = current->declaration;

	error_t *error;
	error = error_create(node->position, message);
	if (!error)
	{
		return NULL;
	}

	if (list_rpush(graph->errors, (list_value_t)error))
	{
		return NULL;
	}

	return error;
}



static int32_t
syntax_symbol_in_object_struct(graph_t *graph, symbol_t *current, symbol_t *target);

static int32_t
syntax_symbol_in_array_struct(graph_t *graph, symbol_t *current, symbol_t *target);


static int32_t
syntax_compare_symbol_id(symbol_t *current, symbol_t *target)
{
	node_t *node_current = current->declaration;
	node_basic_t *node_basic_current;
	node_basic_current = (node_basic_t *)node_current->value;

	node_t *node_target = target->declaration;
	node_basic_t *node_basic_target;
	node_basic_target = (node_basic_t *)node_target->value;

	return (strncmp(node_basic_target->value, node_basic_current->value, 
		max(strlen(node_basic_current->value), strlen(node_basic_target->value))) == 0);
}


static int32_t
syntax_same_symbol_in_name_struct(graph_t *graph, symbol_t *current, symbol_t *target)
{
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if(symbol_check_flag(a, SYMBOL_FLAG_OBJECT))
		{
			return syntax_symbol_in_object_struct(graph, a, target);
		}
		else if(symbol_check_flag(a, SYMBOL_FLAG_ARRAY))
		{
			return syntax_symbol_in_array_struct(graph, a, target);
		}
		else if(symbol_check_flag(a, SYMBOL_FLAG_ID))
		{
			if(a->id == target->id)
			{
				break;
			}
			if(syntax_compare_symbol_id(a, target))
			{
				syntax_error(graph, a, "duplicated symbol(%d) in %lld:%lld\n", a->flags, 
					target->declaration->position.line, target->declaration->position.column);
				return 0;
			}
		}
	}
	return 1;
}

static int32_t
syntax_symbol_in_object_property_struct(graph_t *graph, symbol_t *current, symbol_t *target)
{
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if(symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			return syntax_same_symbol_in_name_struct(graph, a, target);
		}
	}
	return 1;
}

static int32_t
syntax_symbol_in_object_struct(graph_t *graph, symbol_t *current, symbol_t *target)
{
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if(symbol_check_flag(a, SYMBOL_FLAG_OBJECT_PROPERTY))
		{
			return syntax_symbol_in_object_property_struct(graph, a, target);
		}
	}
	return 1;
}

static int32_t
syntax_symbol_in_array_struct(graph_t *graph, symbol_t *current, symbol_t *target)
{
	symbol_t *a;
	for (a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_OBJECT))
		{
			return syntax_symbol_in_object_struct(graph, a, target);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_ARRAY))
		{
			return syntax_symbol_in_array_struct(graph, a, target);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_ID))
		{
			if(syntax_compare_symbol_id(a, target))
			{
				syntax_error(graph, a, "duplicated symbol(%d) in %lld:%lld\n", a->flags, 
					target->declaration->position.line, target->declaration->position.column);
				return 0;
			}
		}
	}
	return 1;
}

static int32_t
syntax_same_symbol_contain_name_struct(graph_t *graph, symbol_t *current, symbol_t *target)
{
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			return syntax_same_symbol_in_name_struct(graph, a, target);
		}
	}

	return 1;
}

static int32_t
syntax_symbol_is_duplicated(graph_t *graph, symbol_t *root, symbol_t *sub, symbol_t *target)
{
	int32_t result = 1;
	symbol_t *a;
	for(a = root->begin; a && (a != sub) && (a != root->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_IMPORT))
		{
			symbol_t *b;
			for(b = a->begin; b != a->end; b = b->next)
			{
				if (symbol_check_flag(b, SYMBOL_FLAG_FIELD))
				{
					return syntax_same_symbol_contain_name_struct(graph, b, target);
				}
			}
			if(!result)
			{
				return 0;
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_FIELD))
		{
			result = syntax_same_symbol_contain_name_struct(graph, a, target);
			if(!result)
			{
				return 0;
			} 
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_TYPE_PARAMETER))
		{
			result = syntax_same_symbol_contain_name_struct(graph, a, target);
			if(!result)
			{
				return 0;
			} 
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_HERITAGE))
		{
			result = syntax_same_symbol_contain_name_struct(graph, a, target);
			if(!result)
			{
				return 0;
			} 
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_VAR))
		{
			result = syntax_same_symbol_contain_name_struct(graph, a, target);
			if(!result)
			{
				return 0;
			} 
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_CONST))
		{
			result = syntax_same_symbol_contain_name_struct(graph, a, target);
			if(!result)
			{
				return 0;
			} 
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_CLASS))
		{
			result = syntax_same_symbol_contain_name_struct(graph, a, target);
			if(!result)
			{
				return 0;
			} 
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_FUNCTION))
		{
			result = syntax_same_symbol_contain_name_struct(graph, a, target);
			if(!result)
			{
				return 0;
			} 
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_TYPE))
		{
			result = syntax_same_symbol_contain_name_struct(graph, a, target);
			if(!result)
			{
				return 0;
			} 
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_ENUM))
		{
			result = syntax_same_symbol_contain_name_struct(graph, a, target);
			if(!result)
			{
				return 0;
			} 
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_PROPERTY))
		{
			result = syntax_same_symbol_contain_name_struct(graph, a, target);
			if(!result)
			{
				return 0;
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_METHOD))
		{
			result = syntax_same_symbol_contain_name_struct(graph, a, target);
			if(!result)
			{
				return 0;
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_EXPORT))
		{
			symbol_t *b;
			for(b = a->begin; b && (b != a->end); b = b->next)
			{
				result = syntax_symbol_is_duplicated(graph, root, a, target);
				if(!result)
				{
					return 0;
				}
			}
		}
	}

	return 1;
}


static int32_t
syntax_object(graph_t *graph, symbol_t *root, symbol_t *sub, symbol_t *current);

static int32_t
syntax_array(graph_t *graph, symbol_t *root, symbol_t *sub, symbol_t *current);


static int32_t
syntax_name(graph_t *graph, symbol_t *root, symbol_t *sub, symbol_t *current)
{
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if(symbol_check_flag(a, SYMBOL_FLAG_OBJECT))
		{
			return syntax_object(graph, root, sub, a);
		}
		else if(symbol_check_flag(a, SYMBOL_FLAG_ARRAY))
		{
			return syntax_array(graph, root, sub, a);
		}
		else if(symbol_check_flag(a, SYMBOL_FLAG_ID))
		{
			return syntax_symbol_is_duplicated(graph, root, sub, a);
		}
	}
	return 0;
}

static int32_t
syntax_object_property(graph_t *graph, symbol_t *root, symbol_t *sub, symbol_t *current)
{
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if(symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			return syntax_name(graph, root, sub, a);
		}
	}
	return 0;
}

static int32_t
syntax_object(graph_t *graph, symbol_t *root, symbol_t *sub, symbol_t *current)
{
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if(symbol_check_flag(a, SYMBOL_FLAG_OBJECT_PROPERTY))
		{
			return syntax_object_property(graph, root, sub, a);
		}
	}
	return 0;
}

static int32_t
syntax_array(graph_t *graph, symbol_t *root, symbol_t *sub, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_OBJECT))
		{
			return syntax_object(graph, root, sub, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_ARRAY))
		{
			return syntax_array(graph, root, sub, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_ID))
		{
			return syntax_symbol_is_duplicated(graph, root, sub, a);
		}
	}
	return 0;
}


static int32_t
syntax_contain_name(graph_t *graph, symbol_t *root, symbol_t *sub, symbol_t *current)
{
	int32_t result = 1;
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			result &= syntax_name(graph, root, sub, a);
		}
	}
	return result;
}

static int32_t
syntax_contain_symbol_by_flag(symbol_t *refrence, uint64_t flag)
{
	symbol_t *a;
	for (a = refrence->begin; a != refrence->end; a = a->next)
	{
		if (symbol_check_flag(a, flag))
		{
			return 1;
		}
	}
	return 0;
}

static int32_t
syntax_is_same_type_parameter(symbol_t *refrence, symbol_t *target)
{
	uint64_t refrence_counter = 0;
	uint64_t refrence_counter_by_value = 0;
	symbol_t *a;
	for (a = refrence->begin; a != refrence->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_TYPE_PARAMETER))
		{
			if (syntax_contain_symbol_by_flag(a, SYMBOL_FLAG_TYPE_PARAMETER_VALUE))
			{
				refrence_counter_by_value += 1;
			}
			refrence_counter += 1;
		}
	}

	uint64_t target_counter = 0;
	uint64_t target_counter_by_value = 0;

	for (a = target->begin; a != target->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_TYPE_PARAMETER))
		{
			if (syntax_contain_symbol_by_flag(a, SYMBOL_FLAG_TYPE_PARAMETER_VALUE))
			{
				target_counter_by_value += 1;
			}
			target_counter += 1;
		}
	}

	if ((refrence_counter - refrence_counter_by_value) == (target_counter - target_counter_by_value))
	{
		return 1;
	}

	return 0;
}



static symbol_t *
syntax_get_prototype_of_type(symbol_t *root, symbol_t *sub, symbol_t *target);

static int32_t
syntax_equal_of_type(symbol_t *refrence, symbol_t *target)
{
	if (!symbol_equal_flag(refrence, target))
	{
		return 0;
	}

	if (symbol_check_flag(refrence, SYMBOL_FLAG_ID))
	{
		if (!syntax_compare_symbol_id(refrence, target))
		{
			return 0;
		}
		return 1;
	}

	uint64_t refrence_counter = 0;
	symbol_t *a;
	for (a = refrence->begin; a != refrence->end; a = a->next)
	{
		refrence_counter += 1;

		int32_t founded = 0;
		uint64_t target_counter = 0;
		symbol_t *b;
		for (b = target->begin;b != target->end;b = b->next)
		{
			target_counter += 1;
			if (target_counter < refrence_counter)
			{
				continue;
			}

			founded = 1;

			if (!symbol_equal_flag(a, b))
			{
				return 0;
			}

			if (symbol_check_flag(a, SYMBOL_FLAG_ID))
			{
				if (!syntax_compare_symbol_id(a, b))
				{
					return 0;
				}
			}
			if (!syntax_equal_of_type(a, b))
			{
				return 0;
			}
			break;
		}
		if (!founded)
		{
			return 0;
		}
	}
	return 1;
}

static symbol_t *
syntax_get_prototype_of_type_in_type_parameter(symbol_t *root, symbol_t *sub, symbol_t *refrence, symbol_t *target)
{
	symbol_t *a;
	for (a = refrence->begin;a != refrence->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			symbol_t *b;
			for (b = a->begin;b != a->end; b = b->next)
			{
				if(syntax_equal_of_type(b, target))
				{
					symbol_t *c;
					for (c = refrence->begin;c != refrence->end; c = c->next)
					{
						if (symbol_check_flag(c, SYMBOL_FLAG_TYPE_PARAMETER_TYPE))
						{
							symbol_t *d;
							for (d = c->begin;d != c->end;d = d->next)
							{
								symbol_t *e;
								e = syntax_get_prototype_of_type(root, a, d);
								if (e)
								{
									return e;
								}
								return d;
							}
						}
					}
				}
			}
		}
	}
	return NULL;
}

static symbol_t *
syntax_get_prototype_of_type_in_heritage(symbol_t *root, symbol_t *sub, symbol_t *refrence, symbol_t *target)
{
	symbol_t *a;
	for (a = refrence->begin;a != refrence->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			symbol_t *b;
			for (b = a->begin;b != a->end; b = b->next)
			{
				if(syntax_equal_of_type(b, target))
				{
					symbol_t *c;
					for (c = refrence->begin;c != refrence->end;c = c->next)
					{
						if (symbol_check_flag(c, SYMBOL_FLAG_HERITAGE_TYPE))
						{
							symbol_t *d;
							for (d = c->begin;d != c->end;d = d->next)
							{
								symbol_t *e;
								e = syntax_get_prototype_of_type(root, a, d);
								if (e)
								{
									return e;
								}
								return d;
							}
						}
					}
				}
			}
		}
	}
	return NULL;
}


static symbol_t *
syntax_get_prototype_of_type(symbol_t *root, symbol_t *sub, symbol_t *target)
{
	if (symbol_check_flag(root, SYMBOL_FLAG_CLASS))
	{
		symbol_t *a;
		for (a = root->begin;(a != sub) && (a != root->end); a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_TYPE_PARAMETER))
			{
				symbol_t *b;
				b = syntax_get_prototype_of_type_in_type_parameter(root, sub, a, target);
				if (b)
				{
					return b;
				}
			}
			else if (symbol_check_flag(a, SYMBOL_FLAG_HERITAGE))
			{
				symbol_t *b = NULL;
				b = syntax_get_prototype_of_type_in_heritage(root, sub, a, target);
				if (b)
				{
					return b;
				}
			}
		}
		if(root->parent)
		{
			return syntax_get_prototype_of_type(root->parent, root, target);
		}
	}
	else if (symbol_check_flag(root, SYMBOL_FLAG_METHOD))
	{
		symbol_t *a;
		for (a = root->begin;(a != sub) && (a != root->end); a = a->next)
		{
			
		}

		if(root->parent)
		{
			return syntax_get_prototype_of_type(root->parent, root, target);
		}
	}
	else if (symbol_check_flag(root, SYMBOL_FLAG_FUNCTION))
	{
		symbol_t *a;
		for (a = root->begin;(a != sub) && (a != root->end); a = a->next)
		{
			
		}

		if(root->parent)
		{
			return syntax_get_prototype_of_type(root->parent, root, target);
		}
	}
	else if (symbol_check_flag(root, SYMBOL_FLAG_MODULE)) 
	{
		symbol_t *a;
		for (a = root->begin;(a != sub) && (a != root->end); a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_TYPE))
			{
				symbol_t *b = NULL;
				//b = syntax_get_prototype_of_type_in_type(a, NULL, target);
				if (b)
				{
					return b;
				}
			}
			else if (symbol_check_flag(a, SYMBOL_FLAG_IMPORT))
			{
				symbol_t *b = NULL;
				//b = syntax_get_prototype_of_type_in_import(a, NULL, target);
				if (b)
				{
					return b;
				}
			}
		}
	}

	if (root->parent)
	{
		return syntax_get_prototype_of_type(root->parent, root, target);
	}

	return NULL;
}

static int32_t
syntax_subset_of_type(symbol_t *refrence, symbol_t *target)
{
	uint64_t refrence_counter = 0;
	symbol_t *a;
	for (a = refrence->begin; a != refrence->end; a = a->next)
	{
		refrence_counter += 1;

		symbol_t *c;
		c = syntax_get_prototype_of_type(refrence, a, a);
		if(!c)
		{
			c = a;
		}

		int32_t founded = 0;
		uint64_t target_counter = 0;
		symbol_t *b;
		for (b = target->begin;b != target->end;b = b->next)
		{
			target_counter += 1;
			if (target_counter < refrence_counter)
			{
				continue;
			}

			founded = 1;

			symbol_t *d;
			d = syntax_get_prototype_of_type(target, b, b);
			if(!d)
			{
				d = b;
			}
			
			if (!symbol_equal_flag(c, d))
			{
				return 0;
			}

			if (symbol_check_flag(c, SYMBOL_FLAG_ID))
			{
				if (!syntax_compare_symbol_id(c, d))
				{
					return 0;
				}
			}
			if (!syntax_subset_of_type(c, d))
			{
				return 0;
			}
			break;
		}
		if (!founded)
		{
			return 0;
		}
	}
	return 1;
}

static int32_t
syntax_subset_of_parameter_type(symbol_t *refrence, symbol_t *target)
{
	uint64_t refrence_counter = 0;
	symbol_t *a;
	for (a = refrence->begin; a != refrence->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_PARAMETER_TYPE))
		{
			refrence_counter += 1;
			int32_t founded = 0;
			uint64_t target_counter = 0;
			symbol_t *b;
			for (b = target->begin; b != target->end; b = b->next)
			{
				if (symbol_check_flag(b, SYMBOL_FLAG_PARAMETER_TYPE))
				{
					target_counter += 1;
					if(target_counter < refrence_counter)
					{
						continue;
					}
					founded = 1;
					if(!syntax_subset_of_type(a, b))
					{
						return 0;
					}
					break;
				}
			}
			if(!founded)
			{
				return 0;
			}
		}
	}
	return 1;
}

static int32_t
syntax_parameter_is_ellipsis(symbol_t *refrence)
{
	symbol_t *a;
	for (a = refrence->begin; a != refrence->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			symbol_t *b;
			for (b = a->begin; b != a->end; b = b->next)
			{
				if (symbol_check_flag(b, SYMBOL_FLAG_ELLIPSIS))
				{
					return 1;
				}
			}
		}
	}
	return 0;
}

static int32_t
syntax_subset_of_array_type(symbol_t *refrence, symbol_t *target)
{
	symbol_t *a;
	for (a = target->begin; a != target->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_GET_ITEM))
		{
			symbol_t *b;
			for (b = a->begin; b != a->end; b = b->next)
			{
				if (symbol_check_flag(b, SYMBOL_FLAG_NAME))
				{
					if(syntax_subset_of_type(refrence, b))
					{
						return 1;
					}
				}
			}
		}
	}
	return 0;
}

static int32_t
syntax_subset_of_ellipsis_type(symbol_t *refrence, symbol_t *target)
{
	symbol_t *a;
	for (a = refrence->begin; a != refrence->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_PARAMETER_TYPE))
		{
			symbol_t *b;
			for (b = target->begin; b != target->end; b = b->next)
			{
				if (symbol_check_flag(b, SYMBOL_FLAG_PARAMETER_TYPE))
				{
					return syntax_subset_of_array_type(a, b);
				}
			}
		}
	}
	return 1;
}

static int32_t
syntax_subset_of_parameter(symbol_t *refrence, symbol_t *target)
{
	uint64_t refrence_parameter_counter = 0;  
	symbol_t *a;
	for (a = refrence->begin; a != refrence->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_PARAMETER))
		{
			if (syntax_contain_symbol_by_flag(a, SYMBOL_FLAG_PARAMETER_VALUE))
			{
				continue;
			}

			refrence_parameter_counter += 1;
			int32_t target_founded = 0;
			uint64_t target_parameter_counter = 0;
			symbol_t *b;
			for (b = target->begin; b != target->end; b = b->next)
			{
				if (symbol_check_flag(b, SYMBOL_FLAG_PARAMETER))
				{
					target_parameter_counter += 1;
					if (target_parameter_counter < refrence_parameter_counter)
					{
						continue;
					}
					target_founded = 1;

					if (syntax_parameter_is_ellipsis(a))
					{
						if (!syntax_subset_of_ellipsis_type(b, a))
						{
							goto not_subset;
						}
						goto subset;
					}

					if (syntax_parameter_is_ellipsis(b))
					{
						if (!syntax_subset_of_ellipsis_type(a, b))
						{
							goto not_subset;
						}
						goto subset;
					}

					if (!syntax_subset_of_parameter_type(a, b))
					{
						goto not_subset;
					}
					break;
				}
			}
			
			if (!target_founded)
			{
				if (syntax_contain_symbol_by_flag(a, SYMBOL_FLAG_PARAMETER_VALUE))
				{
					continue;
				}
				goto not_subset;
			}
		}
	}

	uint64_t target_parameter_counter = 0;
	for (a = target->begin; a != target->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_PARAMETER))
		{
			target_parameter_counter += 1;
			if (target_parameter_counter <= refrence_parameter_counter)
			{
				continue;
			}

			if (syntax_parameter_is_ellipsis(a))
			{
				goto subset;
			}
			
			if (!syntax_contain_symbol_by_flag(a, SYMBOL_FLAG_PARAMETER_VALUE))
			{
				goto not_subset;
			}
		}
	}

	subset:
	return 1;

	not_subset:
	return 0;
}

static int32_t
syntax_method_on_name_struct(graph_t *graph, symbol_t *current, symbol_t *refrence, symbol_t *target_refrence, symbol_t *target)
{
	int32_t result = 1;
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if(symbol_check_flag(a, SYMBOL_FLAG_ID))
		{
			if(a->id == target->id)
			{
				break;
			}
			if(syntax_compare_symbol_id(a, target))
			{
				result = syntax_is_same_type_parameter(refrence, target_refrence);
				if(result)
				{
					result = syntax_subset_of_parameter(refrence, target_refrence);
					if(result)
					{
						goto error;
					}
					result = syntax_subset_of_parameter(target_refrence, refrence);
					if(result)
					{
						goto error;
					}
				}
				return 1;

				error:
				syntax_error(graph, a, "duplicated symbol(%d) in %lld:%lld\n", a->flags, 
					target->declaration->position.line, target->declaration->position.column);
				return 0;
			}
		}
	}
	return 1;
}

static int32_t
syntax_method_contain_name_struct(graph_t *graph, symbol_t *current, symbol_t *refrence, symbol_t *target_refrence, symbol_t *target)
{
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			return syntax_method_on_name_struct(graph, a, refrence, target_refrence, target);
		}
	}
	return 1;
}

static int32_t
syntax_method_is_duplicated(graph_t *graph, symbol_t *root, symbol_t *sub, symbol_t *refrence, symbol_t *target)
{
	int32_t result = 1;
	symbol_t *a;
	for (a = root->begin; a && (a != sub) && (a != root->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_METHOD))
		{
			result = syntax_method_contain_name_struct(graph, a, refrence, a, target);
			if(!result)
			{
				return 0;
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_EXPORT))
		{
			symbol_t *b;
			for (b = a->begin; b && (b != a->end); b = b->next)
			{
				result = syntax_method_is_duplicated(graph, root, a, refrence, target);
				if (!result)
				{
					return 0;
				}
			}
		}
	}
	return 1;
}

static int32_t
syntax_method_name(graph_t *graph, symbol_t *root, symbol_t *sub, symbol_t *refrence, symbol_t *current)
{
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if(symbol_check_flag(a, SYMBOL_FLAG_ID))
		{
			return syntax_method_is_duplicated(graph, root, sub, refrence, a);
		}
	}
	return 0;
}





static int32_t
syntax_import(graph_t *graph, symbol_t *root , symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_FIELD))
		{
			result &= syntax_contain_name(graph, current, a, a);
			if(!result)
			{
				return 0;
			}
			result &= syntax_contain_name(graph, root, current, a);
			if(!result)
			{
				return 0;
			}
		}
	}

	return result;
}

static int32_t
syntax_type_parameter(graph_t *graph, symbol_t *root , symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			result &= syntax_name(graph, root, a, a);
			if(!result)
			{
				return 0;
			}
		}
	}

	return result;
}

static int32_t
syntax_heritage(graph_t *graph, symbol_t *root, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			result &= syntax_name(graph, root, a, a);
			if(!result)
			{
				return 0;
			}
			symbol_t *b;
			for(b = root->begin; b != root->end; b = b->next)
			{
				if (symbol_check_flag(b, SYMBOL_FLAG_TYPE_PARAMETER))
				{
					result &= syntax_contain_name(graph, root, b, b);
					if(!result)
					{
						return result;
					}
				}
			}
		}
	}

	return result;
}

static int32_t
syntax_method(graph_t *graph, symbol_t *root, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for (a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			result &= syntax_method_name(graph, root, current, current, a);
			if (!result)
			{
				return 0;
			}
		}
	}

	return result;
}

static int32_t
syntax_class(graph_t *graph, symbol_t *root, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for (a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			result &= syntax_name(graph, root, current, a);
			if(!result)
			{
				return 0;
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_HERITAGE))
		{
			result &= syntax_heritage(graph, current, a);
			if(!result)
			{
				return 0;
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_TYPE_PARAMETER))
		{
			result &= syntax_type_parameter(graph, current, a);
			if(!result)
			{
				return 0;
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_CLASS))
		{
			symbol_t *b;
			for(b = current->begin; b != current->end; b = b->next)
			{
				if (symbol_check_flag(b, SYMBOL_FLAG_TYPE_PARAMETER))
				{
					result &= syntax_contain_name(graph, current, b, a);
					if(!result)
					{
						return result;
					}
				}
				if (symbol_check_flag(b, SYMBOL_FLAG_HERITAGE))
				{
					result &= syntax_contain_name(graph, current, b, a);
					if(!result)
					{
						return result;
					}
				}
			}
			result &= syntax_class(graph, current, a);
			if(!result)
			{
				return 0;
			}
			if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
			{
				result &= syntax_name(graph, current, a, a);
				if(!result)
				{
					return 0;
				}
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_METHOD))
		{
			symbol_t *b;
			for (b = current->begin; b != current->end; b = b->next)
			{
				if (symbol_check_flag(b, SYMBOL_FLAG_TYPE_PARAMETER))
				{
					result &= syntax_contain_name(graph, current, b, a);
					if(!result)
					{
						return result;
					}
				}
				if (symbol_check_flag(b, SYMBOL_FLAG_HERITAGE))
				{
					result &= syntax_contain_name(graph, current, b, a);
					if(!result)
					{
						return result;
					}
				}
			}
			result &= syntax_method(graph, current, a);
			if (!result)
			{
				return 0;
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_ENUM))
		{
			symbol_t *b;
			for(b = current->begin; b != current->end; b = b->next)
			{
				if (symbol_check_flag(b, SYMBOL_FLAG_TYPE_PARAMETER))
				{
					result &= syntax_contain_name(graph, current, b, a);
					if(!result)
					{
						return result;
					}
				}
				if (symbol_check_flag(b, SYMBOL_FLAG_HERITAGE))
				{
					result &= syntax_contain_name(graph, current, b, a);
					if(!result)
					{
						return result;
					}
				}
			}
			if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
			{
				result &= syntax_name(graph, current, a, a);
				if(!result)
				{
					return 0;
				}
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_PROPERTY))
		{
			symbol_t *b;
			for(b = current->begin; b != current->end; b = b->next)
			{
				if (symbol_check_flag(b, SYMBOL_FLAG_TYPE_PARAMETER))
				{
					result &= syntax_contain_name(graph, current, b, a);
					if(!result)
					{
						return result;
					}
				}
				if (symbol_check_flag(b, SYMBOL_FLAG_HERITAGE))
				{
					result &= syntax_contain_name(graph, current, b, a);
					if(!result)
					{
						return result;
					}
				}
			}
			if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
			{
				result &= syntax_name(graph, current, a, a);
				if(!result)
				{
					return 0;
				}
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_EXPORT))
		{
			symbol_t *b;
			for (b = a->begin; b != a->end; b = b->next)
			{
				result &= syntax_class(graph, root, b);
				if (!result)
				{
					return result;
				}
			}
		}
	}

	return result;
}

static int32_t
syntax_enum(graph_t *graph, symbol_t *root, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			result &= syntax_name(graph, root, current, a);
			if(!result)
			{
				return 0;
			}
		}
	}

	return result;
}

static int32_t
syntax_var(graph_t *graph, symbol_t *root, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			result &= syntax_name(graph, root, current, a);
			if(!result)
			{
				return 0;
			}
		}
	}

	return result;
}

static int32_t
syntax_const(graph_t *graph, symbol_t *root, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			result &= syntax_name(graph, root, current, a);
			if(!result)
			{
				return 0;
			}
		}
	}

	return result;
}

static int32_t
syntax_func(graph_t *graph, symbol_t *root, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			result &= syntax_name(graph, root, current, a);
			if(!result)
			{
				return 0;
			}
		}
	}

	return result;
}

static int32_t
syntax_export(graph_t *graph, symbol_t *root, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_IMPORT))
		{
			result &= syntax_import(graph, root, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_CLASS))
		{
			result &= syntax_class(graph, root, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_ENUM))
		{
			result &= syntax_enum(graph, root, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_VAR))
		{
			result &= syntax_var(graph, root, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_CONST))
		{
			result &= syntax_const(graph, root, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_FUNCTION))
		{
			result &= syntax_func(graph, root, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_EXPORT))
		{
			result &= syntax_export(graph, root, a);
		}
	}

	return result;
}






static int32_t
syntax_module(graph_t *graph, symbol_t *current)
{
	int32_t result = 1;

	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_IMPORT))
		{
			result &= syntax_import(graph, current, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_CLASS))
		{
			result &= syntax_class(graph, current, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_ENUM))
		{
			result &= syntax_enum(graph, current, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_VAR))
		{
			result &= syntax_var(graph, current, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_CONST))
		{
			result &= syntax_const(graph, current, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_FUNCTION))
		{
			result &= syntax_func(graph, current, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_EXPORT))
		{
			result &= syntax_export(graph, current, a);
		}
	}

	return result;
}

int32_t
syntax_run(graph_t *graph)
{
	symbol_t *symbol;
	symbol = (symbol_t *)graph->symbol;

	int32_t result = 1;

	symbol_t *a;
	for(a = symbol->begin; a != symbol->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_MODULE))
		{
			result &= syntax_module(graph, a);
		}
	}

	return result;
}





