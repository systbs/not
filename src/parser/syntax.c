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
syntax_symbol_contain_flag(symbol_t *refrence, uint64_t flag)
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

static symbol_t *
syntax_extract_unary(symbol_t *target, uint64_t flag)
{
	symbol_t *a;
	for (a = target->begin;a != target->end;a = a->next)
	{
		if (symbol_check_flag(a, flag))
		{
			symbol_t *b;
			for (b = a->begin;b != a->end;b = b->next)
			{
				return b;
			}
		}
	}
	return NULL;
}

static symbol_t *
syntax_subset_of_object(symbol_t *refrence, symbol_t *target);

static symbol_t *
syntax_subset_of_array(symbol_t *refrence, symbol_t *target);

static int32_t
syntax_subset_compare_string(symbol_t *refrence, char *name)
{
	node_t *node_refrence = refrence->declaration;
	node_basic_t *node_basic_refrence;
	node_basic_refrence = (node_basic_t *)node_refrence->value;

	return (strncmp(name, node_basic_refrence->value, 
		max(strlen(node_basic_refrence->value), strlen(name))) == 0);
}

static int32_t
syntax_subset_compare_id(symbol_t *refrence, symbol_t *target)
{
	node_t *node_refrence = refrence->declaration;
	node_basic_t *node_basic_refrence;
	node_basic_refrence = (node_basic_t *)node_refrence->value;

	node_t *node_target = target->declaration;
	node_basic_t *node_basic_target;
	node_basic_target = (node_basic_t *)node_target->value;

	return (strncmp(node_basic_target->value, node_basic_refrence->value, 
		max(strlen(node_basic_refrence->value), strlen(node_basic_target->value))) == 0);
}

static symbol_t *
syntax_subset_exist_in_set(symbol_t *refrence, symbol_t *target)
{
	symbol_t *a;
	for (a = refrence->begin;(a != refrence->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_ID))
		{
			int32_t result;
			result = syntax_subset_compare_id(a, target);
			if(result)
			{
				return a;
			}
		}
		else if(symbol_check_flag(a, SYMBOL_FLAG_OBJECT))
		{
			return syntax_subset_of_object(a, target);
		}
		else if(symbol_check_flag(a, SYMBOL_FLAG_ARRAY))
		{
			return syntax_subset_of_array(a, target);
		}
	}
	return NULL;
}

static symbol_t *
syntax_subset_of_object_property(symbol_t *refrence, symbol_t *target)
{
	symbol_t *a;
	for(a = refrence->begin; a != refrence->end; a = a->next)
	{
		if(symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			return syntax_subset_exist_in_set(a, target);
		}
	}
	return NULL;
}

static symbol_t *
syntax_subset_of_object(symbol_t *refrence, symbol_t *target)
{
	symbol_t *a;
	for(a = refrence->begin; a != refrence->end; a = a->next)
	{
		if(symbol_check_flag(a, SYMBOL_FLAG_OBJECT_PROPERTY))
		{
			return syntax_subset_of_object_property(a, target);
		}
	}
	return NULL;
}

static symbol_t *
syntax_subset_of_array(symbol_t *refrence, symbol_t *target)
{
	symbol_t *a;
	for (a = refrence->begin; a != refrence->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_OBJECT))
		{
			return syntax_subset_of_object(a, target);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_ARRAY))
		{
			return syntax_subset_of_array(a, target);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_ID))
		{
			int32_t result;
			result = syntax_subset_compare_id(a, target);
			if (result)
			{
				return a;
			}
		}
	}
	return NULL;
}

static symbol_t *
syntax_subset_exist_in_flag(symbol_t *refrence, symbol_t *target, uint64_t flag)
{
	symbol_t *a;
	for (a = refrence->begin;(a != refrence->end); a = a->next)
	{
		if (symbol_check_flag(a, flag))
		{
			symbol_t *result;
			result = syntax_subset_exist_in_set(a, target);
			if(result)
			{
				return result;
			}
		}
	}
	return NULL;
}


static int32_t
syntax_duplicated(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *target)
{
	symbol_t *a;
	for(a = root->begin; a && (a != root->end); a = a->next)
	{	
		if (symbol_check_flag(a, SYMBOL_FLAG_IMPORT))
		{
			symbol_t *b;
			for(b = a->begin; b != a->end; b = b->next)
			{
				if (symbol_check_flag(b, SYMBOL_FLAG_FIELD))
				{
					symbol_t *result;
					result = syntax_subset_exist_in_flag(b, target, SYMBOL_FLAG_NAME);
					if(result && (result->id != target->id))
					{
						syntax_error(graph, target, "symbol is duplicated, previous symbol in (%lld:%lld)\n", 
							result->declaration->position.line, result->declaration->position.column);
						return 0;
					}
				}
			}
		}
		
		if (symbol_check_flag(a, SYMBOL_FLAG_FIELD))
		{
			symbol_t *result;
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if(result && (result->id != target->id))
			{
				syntax_error(graph, target, "symbol is duplicated, previous symbol in (%lld:%lld)\n", 
					result->declaration->position.line, result->declaration->position.column);
				return 0;
			}
		}
		
		if (symbol_check_flag(a, SYMBOL_FLAG_VAR))
		{
			symbol_t *result;
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if(result && (result->id != target->id))
			{
				syntax_error(graph, target, "symbol is duplicated, previous symbol in (%lld:%lld)\n", 
					result->declaration->position.line, result->declaration->position.column);
				return 0;
			}
		}
		
		if (symbol_check_flag(a, SYMBOL_FLAG_DEFINE))
		{
			symbol_t *result;
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if(result && (result->id != target->id))
			{
				syntax_error(graph, target, "symbol is duplicated, previous symbol in (%lld:%lld)\n", 
					result->declaration->position.line, result->declaration->position.column);
				return 0;
			}
		}
		
		if (symbol_check_flag(a, SYMBOL_FLAG_FUNCTION))
		{
			symbol_t *result;
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if(result && (result->id != target->id))
			{
				syntax_error(graph, target, "symbol is duplicated, previous symbol in (%lld:%lld)\n", 
					result->declaration->position.line, result->declaration->position.column);
				return 0;
			}
		}
		
		if (symbol_check_flag(a, SYMBOL_FLAG_TYPE))
		{
			symbol_t *result;
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if(result && (result->id != target->id))
			{
				syntax_error(graph, target, "symbol is duplicated, previous symbol in (%lld:%lld)\n", 
					result->declaration->position.line, result->declaration->position.column);
				return 0;
			}
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_IF))
		{
			symbol_t *result;
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if(result && (result->id != target->id))
			{
				syntax_error(graph, target, "symbol is duplicated, previous symbol in (%lld:%lld)\n", 
					result->declaration->position.line, result->declaration->position.column);
				return 0;
			}
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_FOR))
		{
			symbol_t *result;
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if(result && (result->id != target->id))
			{
				syntax_error(graph, target, "symbol is duplicated, previous symbol in (%lld:%lld)\n", 
					result->declaration->position.line, result->declaration->position.column);
				return 0;
			}
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_FORIN))
		{
			symbol_t *result;
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if(result && (result->id != target->id))
			{
				syntax_error(graph, target, "symbol is duplicated, previous symbol in (%lld:%lld)\n", 
					result->declaration->position.line, result->declaration->position.column);
				return 0;
			}
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			symbol_t *result;
			result = syntax_subset_exist_in_set(a, target);
			if(result && (result->id != target->id))
			{
				syntax_error(graph, target, "symbol is duplicated, previous symbol in (%lld:%lld)\n", 
					result->declaration->position.line, result->declaration->position.column);
				return 0;
			}
		}

		if (!symbol_check_flag(root, SYMBOL_FLAG_CLASS) && !symbol_check_flag(root, SYMBOL_FLAG_MODULE))
		{
			if (a == subroot)
			{
				break;
			}
		}
		
		if (symbol_check_flag(a, SYMBOL_FLAG_CLASS))
		{
			symbol_t *result;
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if(result && (result->id != target->id))
			{
				syntax_error(graph, target, "symbol is duplicated, previous symbol in (%lld:%lld)\n", 
					result->declaration->position.line, result->declaration->position.column);
				return 0;
			} 
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_ENUM))
		{
			symbol_t *result;
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if(result && (result->id != target->id))
			{
				syntax_error(graph, target, "symbol is duplicated, previous symbol in (%lld:%lld)\n", 
					result->declaration->position.line, result->declaration->position.column);
				return 0;
			}
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_TYPE_PARAMETER))
		{
			symbol_t *result;
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if(result && (result->id != target->id))
			{
				syntax_error(graph, target, "symbol is duplicated, previous symbol in (%lld:%lld)\n", 
					result->declaration->position.line, result->declaration->position.column);
				return 0;
			} 
		}
		
		if (symbol_check_flag(a, SYMBOL_FLAG_HERITAGE))
		{
			symbol_t *result;
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if(result && (result->id != target->id))
			{
				syntax_error(graph, target, "symbol is duplicated, previous symbol in (%lld:%lld)\n", 
					result->declaration->position.line, result->declaration->position.column);
				return 0;
			}
		}
	
		if (symbol_check_flag(a, SYMBOL_FLAG_PROPERTY))
		{
			symbol_t *result;
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if(result && (result->id != target->id))
			{
				syntax_error(graph, target, "symbol is duplicated, previous symbol in (%lld:%lld)\n", 
					result->declaration->position.line, result->declaration->position.column);
				return 0;
			}
		}
		
		if (symbol_check_flag(a, SYMBOL_FLAG_METHOD))
		{
			symbol_t *result;
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if(result && (result->id != target->id))
			{
				syntax_error(graph, target, "symbol is duplicated, previous symbol in (%lld:%lld)\n", 
					result->declaration->position.line, result->declaration->position.column);
				return 0;
			}
		}
	}

	if (root->parent)
	{
		if (symbol_check_flag(root, SYMBOL_FLAG_BLOCK))
		{
			return syntax_duplicated(graph, root->parent, root, target);
		}
	}

	return 1;
}



static int32_t
syntax_duplicated_by_parameter(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *refrence, symbol_t *target);

static int32_t
syntax_duplicated_object(graph_t *graph, symbol_t *root, symbol_t *sub, symbol_t *current);

static int32_t
syntax_duplicated_array(graph_t *graph, symbol_t *root, symbol_t *sub, symbol_t *current);

static int32_t
syntax_duplicated_in_set(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if(symbol_check_flag(a, SYMBOL_FLAG_OBJECT))
		{
			return syntax_duplicated_object(graph, root, subroot, a);
		}
		else if(symbol_check_flag(a, SYMBOL_FLAG_ARRAY))
		{
			return syntax_duplicated_array(graph, root, subroot, a);
		}
		else if(symbol_check_flag(a, SYMBOL_FLAG_ID))
		{
			return syntax_duplicated(graph, root, subroot, a);
		}
	}
	return 0;
}

static int32_t
syntax_duplicated_in_set_by_parameter(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if(symbol_check_flag(a, SYMBOL_FLAG_ID))
		{
			return syntax_duplicated_by_parameter(graph, root, subroot, subroot, a);
		}
	}
	return 0;
}

static int32_t
syntax_duplicated_object_property(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if(symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			return syntax_duplicated_in_set(graph, root, subroot, a);
		}
	}
	return 0;
}

static int32_t
syntax_duplicated_object(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if(symbol_check_flag(a, SYMBOL_FLAG_OBJECT_PROPERTY))
		{
			return syntax_duplicated_object_property(graph, root, subroot, a);
		}
	}
	return 0;
}

static int32_t
syntax_duplicated_array(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_OBJECT))
		{
			return syntax_duplicated_object(graph, root, subroot, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_ARRAY))
		{
			return syntax_duplicated_array(graph, root, subroot, a);
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_ID))
		{
			return syntax_duplicated(graph, root, subroot, a);
		}
	}
	return 0;
}

static int32_t
syntax_duplicated_in_name(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	int32_t result = 1;
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			result &= syntax_duplicated_in_set(graph, root, subroot, a);
		}
	}
	return result;
}



static symbol_t *
syntax_prototype_of_type(symbol_t *root, symbol_t *subroot, symbol_t *target);

static int32_t
syntax_prototype_subset_of_type_parameter(symbol_t *refrence, symbol_t *target)
{
	uint64_t refrence_counter = 0;
	uint64_t refrence_counter_by_value = 0;
	symbol_t *a;
	for (a = refrence->begin; a != refrence->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_TYPE_PARAMETER))
		{
			if (syntax_symbol_contain_flag(a, SYMBOL_FLAG_VALUE))
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
			if (syntax_symbol_contain_flag(a, SYMBOL_FLAG_VALUE))
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

static int32_t
syntax_prototype_subset_of_component_type(symbol_t *refrence, symbol_t *target)
{
	if(symbol_check_flag(refrence, SYMBOL_FLAG_OR))
	{
		int32_t result = 1;
		symbol_t *a;
		for (a = refrence->begin; a != refrence->end; a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_LEFT))
			{
				result |= syntax_prototype_subset_of_component_type(a, target);
			}
			else if (symbol_check_flag(a, SYMBOL_FLAG_RIGHT))
			{
				result |= syntax_prototype_subset_of_component_type(a, target);
			}
		}
		return result;
	}
	else if(symbol_check_flag(refrence, SYMBOL_FLAG_AND))
	{
		int32_t result = 1;
		symbol_t *a;
		for (a = refrence->begin; a != refrence->end; a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_LEFT))
			{
				result &= syntax_prototype_subset_of_component_type(a, target);
			}
			else if (symbol_check_flag(a, SYMBOL_FLAG_RIGHT))
			{
				result &= syntax_prototype_subset_of_component_type(a, target);
			}
		}
		return result;
	}
	else if (symbol_check_flag(refrence, SYMBOL_FLAG_LEFT))
	{
		if(symbol_check_flag(target, SYMBOL_FLAG_OR))
		{
			int32_t result = 1;
			symbol_t *a;
			for (a = target->begin; a != target->end; a = a->next)
			{
				if (symbol_check_flag(a, SYMBOL_FLAG_LEFT))
				{
					result |= syntax_prototype_subset_of_component_type(a, refrence);
				}
				else if (symbol_check_flag(a, SYMBOL_FLAG_RIGHT))
				{
					result |= syntax_prototype_subset_of_component_type(a, refrence);
				}
			}
			return result;
		}
		else if(symbol_check_flag(target, SYMBOL_FLAG_AND))
		{
			int32_t result = 1;
			symbol_t *a;
			for (a = target->begin; a != target->end; a = a->next)
			{
				if (symbol_check_flag(a, SYMBOL_FLAG_LEFT))
				{
					result &= syntax_prototype_subset_of_component_type(a, refrence);
				}
				else if (symbol_check_flag(a, SYMBOL_FLAG_RIGHT))
				{
					result &= syntax_prototype_subset_of_component_type(a, refrence);
				}
			}
			return result;
		}
		else
		{
			symbol_t *a;
			for (a = refrence->begin; a != refrence->end; a = a->next)
			{
				if (!symbol_equal_flag(a, target))
				{
					return 0;
				}
				if (symbol_check_flag(a, SYMBOL_FLAG_ID) || symbol_check_flag(a, SYMBOL_FLAG_NUMBER) || symbol_check_flag(a, SYMBOL_FLAG_CHAR))
				{
					if (!syntax_subset_compare_id(a, target))
					{
						return 0;
					}
					return 1;
				}
			}
		}
	}
	else if (symbol_check_flag(refrence, SYMBOL_FLAG_RIGHT))
	{
		if(symbol_check_flag(target, SYMBOL_FLAG_OR))
		{
			int32_t result = 1;
			symbol_t *a;
			for (a = target->begin; a != target->end; a = a->next)
			{
				if (symbol_check_flag(a, SYMBOL_FLAG_LEFT))
				{
					result |= syntax_prototype_subset_of_component_type(a, refrence);
				}
				else if (symbol_check_flag(a, SYMBOL_FLAG_RIGHT))
				{
					result |= syntax_prototype_subset_of_component_type(a, refrence);
				}
			}
			return result;
		}
		else if(symbol_check_flag(target, SYMBOL_FLAG_AND))
		{
			int32_t result = 1;
			symbol_t *a;
			for (a = target->begin; a != target->end; a = a->next)
			{
				if (symbol_check_flag(a, SYMBOL_FLAG_LEFT))
				{
					result &= syntax_prototype_subset_of_component_type(a, refrence);
				}
				else if (symbol_check_flag(a, SYMBOL_FLAG_RIGHT))
				{
					result &= syntax_prototype_subset_of_component_type(a, refrence);
				}
			}
			return result;
		}
		else
		{
			symbol_t *a;
			for (a = refrence->begin; a != refrence->end; a = a->next)
			{
				if (!symbol_equal_flag(a, target))
				{
					return 0;
				}
				if (symbol_check_flag(a, SYMBOL_FLAG_ID) || symbol_check_flag(a, SYMBOL_FLAG_NUMBER) || symbol_check_flag(a, SYMBOL_FLAG_CHAR))
				{
					if (!syntax_subset_compare_id(a, target))
					{
						return 0;
					}
					return 1;
				}
			}
		}
	}

	if (!symbol_equal_flag(refrence, target))
	{
		return 0;
	}

	if (symbol_check_flag(refrence, SYMBOL_FLAG_ID) || symbol_check_flag(refrence, SYMBOL_FLAG_NUMBER) || symbol_check_flag(refrence, SYMBOL_FLAG_CHAR))
	{
		if (!syntax_subset_compare_id(refrence, target))
		{
			return 0;
		}
		return 1;
	}

	return 0;
}

static int32_t
syntax_prototype_subset_of_type(symbol_t *refrence, symbol_t *target)
{
	if (!symbol_equal_flag(refrence, target))
	{
		if (!syntax_prototype_subset_of_component_type(refrence, target))
		{
			return 0;
		}
		return 1;
	}

	if (symbol_check_flag(refrence, SYMBOL_FLAG_ID) || symbol_check_flag(refrence, SYMBOL_FLAG_NUMBER) || symbol_check_flag(refrence, SYMBOL_FLAG_CHAR))
	{
		if (!syntax_subset_compare_id(refrence, target))
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

		symbol_t *c;
		c = syntax_prototype_of_type(refrence, a, a);
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
			d = syntax_prototype_of_type(target, b, b);
			if(!d)
			{
				d = b;
			}
			
			if (!symbol_equal_flag(c, d))
			{
				if (!syntax_prototype_subset_of_component_type(c, d))
				{
					return 0;
				}
				return 1;
			}

			if (symbol_check_flag(c, SYMBOL_FLAG_ID))
			{
				if (!syntax_subset_compare_id(c, d))
				{
					return 0;
				}
			}

			if (!syntax_prototype_subset_of_type(c, d))
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
syntax_prototype_of_type_in_type_parameter(symbol_t *root, symbol_t *subroot, symbol_t *refrence, symbol_t *target)
{
	symbol_t *a;
	for (a = refrence->begin;a != refrence->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			symbol_t *b;
			for (b = a->begin;b != a->end; b = b->next)
			{
				if(syntax_prototype_subset_of_type(b, target))
				{
					symbol_t *c;
					for (c = refrence->begin;c != refrence->end; c = c->next)
					{
						if (symbol_check_flag(c, SYMBOL_FLAG_TYPE))
						{
							symbol_t *d;
							for (d = c->begin;d != c->end;d = d->next)
							{
								symbol_t *e;
								e = syntax_prototype_of_type(root, a, d);
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
syntax_prototype_of_type_in_heritage(symbol_t *root, symbol_t *subroot, symbol_t *refrence, symbol_t *target)
{
	symbol_t *a;
	for (a = refrence->begin;a != refrence->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			symbol_t *b;
			for (b = a->begin;b != a->end; b = b->next)
			{
				if(syntax_prototype_subset_of_type(b, target))
				{
					symbol_t *c;
					for (c = refrence->begin;c != refrence->end;c = c->next)
					{
						if (symbol_check_flag(c, SYMBOL_FLAG_TYPE))
						{
							symbol_t *d;
							for (d = c->begin;d != c->end;d = d->next)
							{
								symbol_t *e;
								e = syntax_prototype_of_type(root, a, d);
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
syntax_prototype_of_type_in_parameter(symbol_t *root, symbol_t *subroot, symbol_t *refrence, symbol_t *target)
{
	symbol_t *a;
	for (a = refrence->begin;a != refrence->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			symbol_t *b;
			for (b = a->begin;b != a->end; b = b->next)
			{
				if(syntax_prototype_subset_of_type(b, target))
				{
					symbol_t *c;
					for (c = refrence->begin;c != refrence->end; c = c->next)
					{
						if (symbol_check_flag(c, SYMBOL_FLAG_TYPE))
						{
							symbol_t *d;
							for (d = c->begin;d != c->end;d = d->next)
							{
								symbol_t *e;
								e = syntax_prototype_of_type(root, a, d);
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
syntax_prototype_of_type(symbol_t *root, symbol_t *subset, symbol_t *target)
{
	if (symbol_check_flag(root, SYMBOL_FLAG_CLASS))
	{
		symbol_t *a;
		for (a = root->begin;(a != subset) && (a != root->end); a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_TYPE_PARAMETER))
			{
				symbol_t *b;
				b = syntax_prototype_of_type_in_type_parameter(root, subset, a, target);
				if (b)
				{
					return b;
				}
			}

			if (symbol_check_flag(a, SYMBOL_FLAG_HERITAGE))
			{
				symbol_t *b = NULL;
				b = syntax_prototype_of_type_in_heritage(root, subset, a, target);
				if (b)
				{
					return b;
				}
			}
		}

		if(root->parent)
		{
			return syntax_prototype_of_type(root->parent, root, target);
		}
	}
	
	if (symbol_check_flag(root, SYMBOL_FLAG_METHOD))
	{
		symbol_t *a;
		for (a = root->begin;(a != subset) && (a != root->end); a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_TYPE_PARAMETER))
			{
				symbol_t *b;
				b = syntax_prototype_of_type_in_type_parameter(root, subset, a, target);
				if (b)
				{
					return b;
				}
			}
		}

		if(root->parent)
		{
			return syntax_prototype_of_type(root->parent, root, target);
		}
	}
	
	if (symbol_check_flag(root, SYMBOL_FLAG_FUNCTION))
	{
		symbol_t *a;
		for (a = root->begin;(a != subset) && (a != root->end); a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_TYPE_PARAMETER))
			{
				symbol_t *b;
				b = syntax_prototype_of_type_in_type_parameter(root, subset, a, target);
				if (b)
				{
					return b;
				}
			}
		}

		if(root->parent)
		{
			return syntax_prototype_of_type(root->parent, root, target);
		}
	}
	
	if (symbol_check_flag(root, SYMBOL_FLAG_TYPE))
	{
		symbol_t *a;
		for (a = root->begin;(a != subset) && (a != root->end); a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_TYPE_PARAMETER))
			{
				symbol_t *b;
				b = syntax_prototype_of_type_in_type_parameter(root, subset, a, target);
				if (b)
				{
					return b;
				}
			}
			if (symbol_check_flag(a, SYMBOL_FLAG_HERITAGE))
			{
				symbol_t *b = NULL;
				b = syntax_prototype_of_type_in_heritage(root, subset, a, target);
				if (b)
				{
					return b;
				}
			}
		}
		if(root->parent)
		{
			return syntax_prototype_of_type(root->parent, root, target);
		}
	}

	if (root->parent)
	{
		return syntax_prototype_of_type(root->parent, root, target);
	}

	return NULL;
}




static int32_t
syntax_prototype_of_parameter_type(symbol_t *refrence, symbol_t *target)
{
	uint64_t refrence_counter = 0;
	symbol_t *a;
	for (a = refrence->begin; a != refrence->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_TYPE))
		{
			refrence_counter += 1;
			int32_t founded = 0;
			uint64_t target_counter = 0;
			symbol_t *b;
			for (b = target->begin; b != target->end; b = b->next)
			{
				if (symbol_check_flag(b, SYMBOL_FLAG_TYPE))
				{
					target_counter += 1;
					if(target_counter < refrence_counter)
					{
						continue;
					}
					founded = 1;
					if(!syntax_prototype_subset_of_type(a, b))
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
syntax_prototype_parameter_is_ellipsis(symbol_t *refrence)
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
syntax_prototype_of_array(symbol_t *refrence, symbol_t *target)
{
	if (symbol_check_flag(target, SYMBOL_FLAG_ITEM))
	{
		symbol_t *a;
		for (a = target->begin; a != target->end; a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
			{
				if(syntax_prototype_subset_of_type(refrence, a))
				{
					return 1;
				}
			}
		}
	}

	return 0;
}

static int32_t
syntax_prototype_of_ellipsis(symbol_t *refrence, symbol_t *target)
{
	symbol_t *a;
	a = syntax_extract_unary(refrence, SYMBOL_FLAG_TYPE);
	if (!a)
	{
		return 0;
	}

	symbol_t *b;
	b = syntax_extract_unary(target, SYMBOL_FLAG_TYPE);
	if (!b)
	{
		return 0;
	}

	return syntax_prototype_of_array(a, b);
}

static int32_t
syntax_prototype_of_parameter(symbol_t *refrence, symbol_t *target)
{
	uint64_t refrence_parameter_counter = 0;  
	symbol_t *a;
	for (a = refrence->begin; a != refrence->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_PARAMETER))
		{
			if (syntax_symbol_contain_flag(a, SYMBOL_FLAG_VALUE))
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

					if (syntax_prototype_parameter_is_ellipsis(a))
					{
						if (!syntax_prototype_of_ellipsis(b, a))
						{
							goto not_subset;
						}
						goto subset;
					}

					if (syntax_prototype_parameter_is_ellipsis(b))
					{
						if (!syntax_prototype_of_ellipsis(a, b))
						{
							goto not_subset;
						}
						goto subset;
					}

					if (!syntax_prototype_of_parameter_type(a, b))
					{
						goto not_subset;
					}
					break;
				}
			}
			
			if (!target_founded)
			{
				if (syntax_symbol_contain_flag(a, SYMBOL_FLAG_VALUE))
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

			if (syntax_prototype_parameter_is_ellipsis(a))
			{
				goto subset;
			}
			
			if (!syntax_symbol_contain_flag(a, SYMBOL_FLAG_VALUE))
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
syntax_method_exist_in_set(graph_t *graph, symbol_t *current, symbol_t *refrence, symbol_t *target_refrence, symbol_t *target)
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
			if(syntax_subset_compare_id(a, target))
			{
				result = syntax_prototype_subset_of_type_parameter(refrence, target_refrence);
				if(result)
				{
					result = syntax_prototype_of_parameter(refrence, target_refrence);
					if(result)
					{
						goto error;
					}
					result = syntax_prototype_of_parameter(target_refrence, refrence);
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
syntax_method_exist_in_flag(graph_t *graph, symbol_t *current, symbol_t *refrence, symbol_t *target_refrence, symbol_t *target)
{
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			return syntax_method_exist_in_set(graph, a, refrence, target_refrence, target);
		}
	}
	return 1;
}

static int32_t
syntax_duplicated_by_parameter(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *refrence, symbol_t *target)
{
	int32_t result = 1;
	symbol_t *a;
	for (a = root->begin; a && (a != subroot) && (a != root->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_METHOD))
		{
			result = syntax_method_exist_in_flag(graph, a, refrence, a, target);
			if(!result)
			{
				return 0;
			}
		}
	}
	return 1;
}


static symbol_t *
syntax_locate(symbol_t *root, symbol_t *subroot, symbol_t *target)
{
	symbol_t *result;
	result = syntax_subset_exist_in_flag(root, target, SYMBOL_FLAG_NAME);
	if (result)
	{
		return root;
	}

	symbol_t *a;
	for (a = root->begin;(a != root->end);a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_IMPORT))
		{
			symbol_t *b;
			for(b = a->begin; b != a->end; b = b->next)
			{
				if (symbol_check_flag(b, SYMBOL_FLAG_FIELD))
				{
					symbol_t *result;
					result = syntax_subset_exist_in_flag(b, target, SYMBOL_FLAG_NAME);
					if(result && (result->id != target->id))
					{
						return b;
					}
				}
			}
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_VAR))
		{
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if (result && (result->id != target->id))
			{
				return a;
			}
		}
		
		if (symbol_check_flag(a, SYMBOL_FLAG_PARAMETER))
		{
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if (result && (result->id != target->id))
			{
				return a;
			}
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_DEFINE))
		{
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if (result && (result->id != target->id))
			{
				return a;
			}
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_IF))
		{
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if (result && (result->id != target->id))
			{
				return a;
			}
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_FOR))
		{
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if (result && (result->id != target->id))
			{
				return a;
			}
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_FORIN))
		{
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if (result && (result->id != target->id))
			{
				return a;
			}
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_FIELD))
		{
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if (result && (result->id != target->id))
			{
				return a;
			}
		}

		if (!symbol_check_flag(root, SYMBOL_FLAG_CLASS) && !symbol_check_flag(root, SYMBOL_FLAG_MODULE))
		{
			if (a == subroot)
			{
				break;
			}
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_CLASS))
		{
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if (result && (result->id != target->id))
			{
				return a;
			}
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_METHOD))
		{
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if (result && (result->id != target->id))
			{
				return a;
			}
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_ENUM))
		{
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if (result && (result->id != target->id))
			{
				return a;
			}
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_PROPERTY))
		{
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if (result && (result->id != target->id))
			{
				return a;
			}
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_HERITAGE))
		{
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if (result && (result->id != target->id))
			{
				return a;
			}
		}
	}
	
	if(root->parent)
	{
		result = syntax_locate(root->parent, root, target);
		if(result)
		{
			return result;
		}
	}

	return NULL;
}



static symbol_t *
syntax_find_connectable(symbol_t *root, symbol_t *target);

static int32_t
syntax_prototype_of_ellipsis_by_type(symbol_t *refrence, symbol_t *target)
{
	symbol_t *type;
	type = syntax_extract_unary(target, SYMBOL_FLAG_TYPE);
	if (!type)
	{
		return 0;
	}

	return syntax_prototype_of_array(refrence, type);
}

static int32_t
syntax_equivalent_type_parameter_by_argument(symbol_t *refrence, symbol_t *target)
{
	symbol_t *type;
	type = syntax_extract_unary(refrence, SYMBOL_FLAG_TYPE);
	if (type)
	{
		symbol_t *a;
		for (a = target->begin;a != target->end;a = a->next)
		{
			if (syntax_prototype_subset_of_type(a, type))
			{
				return 1;
			}
		}
		return 0;
	}
	return 0;
}

static int32_t
syntax_find_connectable_match_composite(symbol_t *refrence, symbol_t *target)
{
	uint64_t ref_counter = 0;
	uint64_t tar_counter = 0;
	uint64_t tar_counter_num = 0;
	symbol_t *a;
	for (a = refrence->begin;a != refrence->end;a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_TYPE_PARAMETER))
		{
			int32_t found = 0;
			ref_counter += 1;
			symbol_t *b;
			for (b = target->begin;b != target->end;b = b->next)
			{
				if (symbol_check_flag(b, SYMBOL_FLAG_ARGUMENT))
				{
					tar_counter += 1;
					if (ref_counter > tar_counter)
					{
						continue;
					}
					found = 1;
					if (!syntax_equivalent_type_parameter_by_argument(a, b))
					{
							symbol_t *value;
							value = syntax_extract_unary(a, SYMBOL_FLAG_VALUE);
							if (value)
							{
								tar_counter_num += 1;
								break;
							}
						return 0;
					}
					tar_counter_num += 1;
					break;
				}
			}
			if (tar_counter_num > ref_counter)
			{
				return 0;
			}
			if (!found)
			{
				symbol_t *value;
				value = syntax_extract_unary(a, SYMBOL_FLAG_VALUE);
				if (!value)
				{
					return 0;
				}
			}
		}
	}
	return 1;
}

static symbol_t *
syntax_find_connectable_by_id(symbol_t *root, symbol_t *target)
{
	if (symbol_check_flag(root, SYMBOL_FLAG_CLASS))
	{
		symbol_t *result;
		result = syntax_subset_exist_in_flag(root, target, SYMBOL_FLAG_NAME);
		if (result)
		{
			return root;
		}
	}

	if (symbol_check_flag(root, SYMBOL_FLAG_TYPE))
	{
		symbol_t *result;
		result = syntax_subset_exist_in_flag(root, target, SYMBOL_FLAG_NAME);
		if (result)
		{
			return root;
		}
	}

	symbol_t *a;
	for (a = root->begin;(a != root->end);a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_IMPORT))
		{
			symbol_t *b;
			for(b = a->begin; b != a->end; b = b->next)
			{
				if (symbol_check_flag(b, SYMBOL_FLAG_FIELD))
				{
					symbol_t *result;
					result = syntax_subset_exist_in_flag(b, target, SYMBOL_FLAG_NAME);
					if(result && (result->id != target->id))
					{
						return b;
					}
				}
			}
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_CLASS))
		{
			symbol_t *result;
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if (result && (result->id != target->id))
			{
				return a;
			}
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_TYPE))
		{
			symbol_t *result;
			result = syntax_subset_exist_in_flag(a, target, SYMBOL_FLAG_NAME);
			if (result && (result->id != target->id))
			{
				return a;
			}
		}
	}
	
	if(root->parent)
	{
		symbol_t *result;
		result = syntax_find_connectable_by_id(root->parent, target);
		if(result)
		{
			return result;
		}
	}

	return NULL;
}

static symbol_t *
syntax_find_connectable_by_attribute(symbol_t *root, symbol_t *target)
{
	symbol_t *direct;
	direct = syntax_extract_unary(target, SYMBOL_FLAG_LEFT);

	symbol_t *origin;
	origin = syntax_find_connectable(root, direct);
	if(!origin)
	{
		return NULL;
	}

	symbol_t *name;
	name = syntax_extract_unary(target, SYMBOL_FLAG_RIGHT);

	if(symbol_check_flag(origin, SYMBOL_FLAG_CLASS))
	{
		symbol_t *a;
		for(a = origin->begin; a != origin->end; a = a->next)
		{
			if(symbol_check_flag(a, SYMBOL_FLAG_CLASS))
			{
				if (syntax_subset_exist_in_flag(a, name, SYMBOL_FLAG_NAME))
				{
					node_t *node = a->declaration;
					node_class_t *node_class = (node_class_t *)node->value;
					if ((node_class->flag & PARSER_MODIFIER_EXPORT) == PARSER_MODIFIER_EXPORT)
					{
						return a;
					}
				}
			}
		}
	}

	return NULL;
}

static symbol_t *
syntax_find_connectable_by_composite(symbol_t *root, symbol_t *target)
{
	symbol_t *direct;
	direct = syntax_extract_unary(target, SYMBOL_FLAG_NAME);

	symbol_t *origin;
	origin = syntax_find_connectable(root, direct);
	if(!origin)
	{
		return NULL;
	}

	if (symbol_check_flag(origin, SYMBOL_FLAG_CLASS))
	{
		if (syntax_find_connectable_match_composite(origin, target))
		{
			return origin;
		}
	}
	
	return NULL;
}

static symbol_t *
syntax_find_connectable(symbol_t *root, symbol_t *target)
{
	if (symbol_check_flag(target, SYMBOL_FLAG_COMPOSITE))
	{
		return syntax_find_connectable_by_composite(root, target);
	}

	if (symbol_check_flag(target, SYMBOL_FLAG_ATTR))
	{
		return syntax_find_connectable_by_attribute(root, target);
	}

	if (symbol_check_flag(target, SYMBOL_FLAG_ID))
	{
		return syntax_find_connectable_by_id(root, target);
	}

	return NULL;
}




static symbol_t *
syntax_expression(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current);

static symbol_t *
syntax_postfix(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current);

static symbol_t *
syntax_bitwise_or(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current);

static symbol_t *
syntax_block(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current);

static symbol_t *
syntax_function(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current);

static symbol_t *
syntax_id(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *result;
	result = syntax_locate(root, subroot, current);
	if (!result)
	{
		syntax_error(graph, current, "symbol(%d) already not defined\n", current->flags);
		return NULL;
	}
	return result;
}

static symbol_t *
syntax_parenthesis(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		symbol_t *result;
		result = syntax_expression(graph, current, a, a);
		if(!result)
		{
			return NULL;
		}
	}
	return current;
}

static symbol_t *
syntax_array(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		symbol_t *result;
		result = syntax_expression(graph, current, a, a);
		if(!result)
		{
			return NULL;
		}
	}
	return current;
}

static symbol_t *
syntax_object_property(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				symbol_t *result;
				result = syntax_expression(graph, a, b, b);
				if(!result)
				{
					return NULL;
				}
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_VALUE))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				symbol_t *result;
				result = syntax_expression(graph, a, b, b);
				if(!result)
				{
					return NULL;
				}
			}
		}
	}
	return current;
}

static symbol_t *
syntax_object(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_OBJECT_PROPERTY))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				symbol_t *result;
				result = syntax_object_property(graph, a, b, b);
				if(!result)
				{
					return NULL;
				}
			}
		}
	}
	return current;
}

static symbol_t *
syntax_primary(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *result = NULL;
	if (symbol_check_flag(current, SYMBOL_FLAG_ID))
	{
		result = syntax_id(graph, root, subroot, current);
	}
	else if (symbol_check_flag(current, SYMBOL_FLAG_ARRAY))
	{
		result = syntax_array(graph, root, subroot, current);
	}
	else if (symbol_check_flag(current, SYMBOL_FLAG_OBJECT))
	{
		result = syntax_object(graph, root, subroot, current);
	}
	else if (symbol_check_flag(current, SYMBOL_FLAG_PARENTHESIS))
	{
		result = syntax_parenthesis(graph, root, subroot, current);
	}
	else if (symbol_check_flag(current, SYMBOL_FLAG_FUNCTION))
	{
		result = syntax_function(graph, root, subroot, current);
	}
	else 
	{
		result = current;
	}
	return result;
}

static symbol_t *
syntax_composite(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				symbol_t *result;
				result = syntax_postfix(graph, a, b, b);
				if(!result)
				{
					return NULL;
				}
			}
		}
		if (symbol_check_flag(a, SYMBOL_FLAG_ARGUMENT))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				symbol_t *result;
				result = syntax_expression(graph, a, b, b);
				if(!result)
				{
					return NULL;
				}
			}
		}
	}
	return current;
}

static symbol_t *
syntax_call(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				symbol_t *result;
				result = syntax_postfix(graph, a, b, b);
				if(!result)
				{
					return NULL;
				}
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_ARGUMENT))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				symbol_t *result;
				result = syntax_expression(graph, a, b, b);
				if(!result)
				{
					return NULL;
				}
			}
		}
	}
	return current;
}

static symbol_t *
syntax_get_slice(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				symbol_t *result;
				result = syntax_postfix(graph, a, b, b);
				if(!result)
				{
					return NULL;
				}
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_GET_START))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				symbol_t *result;
				result = syntax_expression(graph, a, b, b);
				if(!result)
				{
					return NULL;
				}
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_GET_STEP))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				symbol_t *result;
				result = syntax_expression(graph, a, b, b);
				if(!result)
				{
					return NULL;
				}
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_GET_STOP))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				symbol_t *result;
				result = syntax_expression(graph, a, b, b);
				if(!result)
				{
					return NULL;
				}
			}
		}
	}
	return current;
}

static symbol_t *
syntax_get_item(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				symbol_t *result;
				result = syntax_postfix(graph, a, b, b);
				if(!result)
				{
					return NULL;
				}
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_INDEX))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				symbol_t *result;
				result = syntax_expression(graph, a, b, b);
				if(!result)
				{
					return NULL;
				}
			}
		}
	}
	return current;
}

static symbol_t *
syntax_get_attr(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *left, *right;
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_LEFT))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				left = syntax_postfix(graph, a, b, b);
				if(!left)
				{
					return NULL;
				}
			}
		}
		
		if (symbol_check_flag(a, SYMBOL_FLAG_RIGHT))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				right = syntax_id(graph, a, b, b);
				if(!right)
				{
					return right;
				}
			}
		}
	}

	return current;
}

static symbol_t *
syntax_postfix(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *result = NULL;
	if (symbol_check_flag(current, SYMBOL_FLAG_COMPOSITE))
	{
		result = syntax_composite(graph, root, subroot, current);
	}
	else if (symbol_check_flag(current, SYMBOL_FLAG_CALL))
	{
		result = syntax_call(graph, root, subroot, current);
	}
	else if (symbol_check_flag(current, SYMBOL_FLAG_SLICE))
	{
		result = syntax_get_slice(graph, root, subroot, current);
	}
	else if (symbol_check_flag(current, SYMBOL_FLAG_ITEM))
	{
		result = syntax_get_item(graph, root, subroot, current);
	}
	else if (symbol_check_flag(current, SYMBOL_FLAG_ATTR))
	{
		result = syntax_get_attr(graph, root, subroot, current);
	}
	else
	{
		result = syntax_primary(graph, root, subroot, current);
	}
	return result;
}

static symbol_t *
syntax_prefix(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	if (
		symbol_check_flag(current, SYMBOL_FLAG_TILDE) || 
		symbol_check_flag(current, SYMBOL_FLAG_NOT) ||
		symbol_check_flag(current, SYMBOL_FLAG_NEG) ||
		symbol_check_flag(current, SYMBOL_FLAG_POS) ||
		symbol_check_flag(current, SYMBOL_FLAG_GET_VALUE) ||
		symbol_check_flag(current, SYMBOL_FLAG_GET_ADDRESS) ||
		symbol_check_flag(current, SYMBOL_FLAG_AWAIT) ||
		symbol_check_flag(current, SYMBOL_FLAG_SIZEOF) ||
		symbol_check_flag(current, SYMBOL_FLAG_TYPEOF) ||
		symbol_check_flag(current, SYMBOL_FLAG_ELLIPSIS))
	{
		symbol_t *a;
		for (a = current->begin;(a != current->end); a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_LEFT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					if (
						symbol_check_flag(b, SYMBOL_FLAG_TILDE) || 
						symbol_check_flag(b, SYMBOL_FLAG_NOT) ||
						symbol_check_flag(b, SYMBOL_FLAG_NEG) ||
						symbol_check_flag(b, SYMBOL_FLAG_POS) ||
						symbol_check_flag(b, SYMBOL_FLAG_GET_VALUE) ||
						symbol_check_flag(b, SYMBOL_FLAG_GET_ADDRESS) ||
						symbol_check_flag(b, SYMBOL_FLAG_AWAIT) ||
						symbol_check_flag(b, SYMBOL_FLAG_SIZEOF) ||
						symbol_check_flag(b, SYMBOL_FLAG_TYPEOF) ||
						symbol_check_flag(b, SYMBOL_FLAG_ELLIPSIS))
					{
						symbol_t *result;
						result = syntax_prefix(graph, a, b, b);
						if(!result)
						{
							return NULL;
						}
					}
					else
					{
						symbol_t *result;
						result = syntax_postfix(graph, a, b, b);
						if(!result)
						{
							return NULL;
						}
					}
				}
			}
			else if (symbol_check_flag(a, SYMBOL_FLAG_RIGHT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					symbol_t *result;
					result = syntax_postfix(graph, a, b, b);
					if(!result)
					{
						return NULL;
					}
				}
			}
		}
	}
	else
	{
		symbol_t *result;
		result = syntax_postfix(graph, root, subroot, current);
		if(!result)
		{
			return NULL;
		}
		return result;
	}
	return current;
}

static symbol_t *
syntax_multiplicative(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	if (
		symbol_check_flag(current, SYMBOL_FLAG_MUL) || 
		symbol_check_flag(current, SYMBOL_FLAG_DIV) ||
		symbol_check_flag(current, SYMBOL_FLAG_MOD))
	{
		symbol_t *a;
		for (a = current->begin;(a != current->end); a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_LEFT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					if (
						symbol_check_flag(b, SYMBOL_FLAG_MUL) || 
						symbol_check_flag(b, SYMBOL_FLAG_DIV) ||
						symbol_check_flag(b, SYMBOL_FLAG_MOD))
					{
						symbol_t *result;
						result = syntax_multiplicative(graph, a, b, b);
						if(!result)
						{
							return NULL;
						}
					}
					else
					{
						symbol_t *result;
						result = syntax_prefix(graph, a, b, b);
						if(!result)
						{
							return NULL;
						}
					}
				}
			}
			else if (symbol_check_flag(a, SYMBOL_FLAG_RIGHT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					symbol_t *result;
					result = syntax_prefix(graph, a, b, b);
					if(!result)
					{
						return NULL;
					}
				}
			}
		}
	}
	else
	{
		symbol_t *result;
		result = syntax_prefix(graph, root, subroot, current);
		if(!result)
		{
			return NULL;
		}
		return result;
	}
	return current;
}

static symbol_t *
syntax_addative(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	if (
		symbol_check_flag(current, SYMBOL_FLAG_PLUS) || 
		symbol_check_flag(current, SYMBOL_FLAG_MINUS))
	{
		symbol_t *a;
		for (a = current->begin;(a != current->end); a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_LEFT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					if (
						symbol_check_flag(b, SYMBOL_FLAG_PLUS) || 
						symbol_check_flag(b, SYMBOL_FLAG_MINUS))
					{
						symbol_t *result;
						result = syntax_addative(graph, a, b, b);
						if(!result)
						{
							return NULL;
						}
					}
					else
					{
						symbol_t *result;
						result = syntax_multiplicative(graph, a, b, b);
						if(!result)
						{
							return NULL;
						}
					}
				}
			}
			else if (symbol_check_flag(a, SYMBOL_FLAG_RIGHT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					symbol_t *result;
					result = syntax_multiplicative(graph, a, b, b);
					if(!result)
					{
						return NULL;
					}
				}
			}
		}
	}
	else
	{
		symbol_t *result;
		result = syntax_multiplicative(graph, root, subroot, current);
		if(!result)
		{
			return NULL;
		}
		return result;
	}
	return current;
}

static symbol_t *
syntax_shifting(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	if (
		symbol_check_flag(current, SYMBOL_FLAG_SHL) || 
		symbol_check_flag(current, SYMBOL_FLAG_SHR))
	{
		symbol_t *a;
		for (a = current->begin;(a != current->end); a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_LEFT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					if (
						symbol_check_flag(b, SYMBOL_FLAG_SHL) || 
						symbol_check_flag(b, SYMBOL_FLAG_SHR))
					{
						symbol_t *result;
						result = syntax_shifting(graph, a, b, b);
						if(!result)
						{
							return NULL;
						}
					}
					else
					{
						symbol_t *result;
						result = syntax_addative(graph, a, b, b);
						if(!result)
						{
							return NULL;
						}
					}
				}
			}
			else if (symbol_check_flag(a, SYMBOL_FLAG_RIGHT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					symbol_t *result;
					result = syntax_addative(graph, a, b, b);
					if(!result)
					{
						return NULL;
					}
				}
			}
		}
	} else {
		symbol_t *result;
		result = syntax_addative(graph, root, subroot, current);
		if(!result)
		{
			return NULL;
		}
		return result;
	}
	return current;
}

static symbol_t *
syntax_relational(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	if (
		symbol_check_flag(current, SYMBOL_FLAG_LT) || 
		symbol_check_flag(current, SYMBOL_FLAG_GT) || 
		symbol_check_flag(current, SYMBOL_FLAG_LE) ||
		symbol_check_flag(current, SYMBOL_FLAG_GE))
	{
		symbol_t *a;
		for (a = current->begin;(a != current->end); a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_LEFT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					if (
						symbol_check_flag(b, SYMBOL_FLAG_LT) || 
						symbol_check_flag(b, SYMBOL_FLAG_GT) || 
						symbol_check_flag(b, SYMBOL_FLAG_LE) ||
						symbol_check_flag(b, SYMBOL_FLAG_GE))
					{
						symbol_t *result;
						result = syntax_relational(graph, a, b, b);
						if(!result)
						{
							return NULL;
						}
					}
					else
					{
						symbol_t *result;
						result = syntax_shifting(graph, a, b, b);
						if(!result)
						{
							return NULL;
						}
					}
				}
			}
			else if (symbol_check_flag(a, SYMBOL_FLAG_RIGHT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					symbol_t *result;
					result = syntax_shifting(graph, a, b, b);
					if(!result)
					{
						return NULL;
					}
				}
			}
		}
	} else {
		symbol_t *result;
		result = syntax_shifting(graph, root, subroot, current);
		if(!result)
		{
			return NULL;
		}
		return result;
	}
	return current;
}

static symbol_t *
syntax_equality(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	if (
		symbol_check_flag(current, SYMBOL_FLAG_EQ) || 
		symbol_check_flag(current, SYMBOL_FLAG_IN) || 
		symbol_check_flag(current, SYMBOL_FLAG_NEQ) ||
		symbol_check_flag(current, SYMBOL_FLAG_EXTENDS))
	{
		symbol_t *a;
		for (a = current->begin;(a != current->end); a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_LEFT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					if (
						symbol_check_flag(b, SYMBOL_FLAG_EQ) || 
						symbol_check_flag(b, SYMBOL_FLAG_IN) || 
						symbol_check_flag(b, SYMBOL_FLAG_NEQ))
					{
						symbol_t *result;
						result = syntax_equality(graph, a, b, b);
						if(!result)
						{
							return NULL;
						}
					}
					else
					{
						symbol_t *result;
						result = syntax_relational(graph, a, b, b);
						if(!result)
						{
							return NULL;
						}
					}
				}
			}
			else if (symbol_check_flag(a, SYMBOL_FLAG_RIGHT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					symbol_t *result;
					result = syntax_relational(graph, a, b, b);
					if(!result)
					{
						return NULL;
					}
				}
			}
		}
	}
	else
	{
		symbol_t *result;
		result = syntax_relational(graph, root, subroot, current);
		if(!result)
		{
			return NULL;
		}
		return result;
	}
	return current;
}

static symbol_t *
syntax_bitwise_and(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	if (symbol_check_flag(current, SYMBOL_FLAG_AND))
	{
		symbol_t *a;
		for (a = current->begin;(a != current->end); a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_LEFT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					if (symbol_check_flag(b, SYMBOL_FLAG_AND))
					{
						symbol_t *result;
						result = syntax_bitwise_and(graph, a, b, b);
						if(!result)
						{
							return NULL;
						}
					}
					else
					{
						symbol_t *result;
						result = syntax_equality(graph, a, b, b);
						if(!result)
						{
							return NULL;
						}
					}
				}
			}
			else if (symbol_check_flag(a, SYMBOL_FLAG_RIGHT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					symbol_t *result;
					result = syntax_equality(graph, a, b, b);
					if(!result)
					{
						return NULL;
					}
				}
			}
		}
	}
	else
	{
		symbol_t *result;
		result = syntax_equality(graph, root, subroot, current);
		if(!result)
		{
			return NULL;
		}
		return result;
	}
	return current;
}

static symbol_t *
syntax_bitwise_xor(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	if (symbol_check_flag(current, SYMBOL_FLAG_XOR))
	{
		symbol_t *a;
		for (a = current->begin;(a != current->end); a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_LEFT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					if (symbol_check_flag(b, SYMBOL_FLAG_XOR))
					{
						symbol_t *result;
						result = syntax_bitwise_xor(graph, a, b, b);
						if(!result)
						{
							return NULL;
						}
					}
					else
					{
						symbol_t *result;
						result = syntax_bitwise_and(graph, a, b, b);
						if(!result)
						{
							return NULL;
						}
					}
				}
			}
			else if (symbol_check_flag(a, SYMBOL_FLAG_RIGHT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					symbol_t *result;
					result = syntax_bitwise_and(graph, a, b, b);
					if(!result)
					{
						return NULL;
					}
				}
			}	
		}
	}
	else
	{
		symbol_t *result;
		result = syntax_bitwise_and(graph, root, subroot, current);
		if(!result)
		{
			return NULL;
		}
		return result;
	}
	return current;
}

static symbol_t *
syntax_bitwise_or(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	if (symbol_check_flag(current, SYMBOL_FLAG_OR))
	{
		symbol_t *a;
		for (a = current->begin;(a != current->end); a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_LEFT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					if (symbol_check_flag(b, SYMBOL_FLAG_OR))
					{
						symbol_t *result;
						result = syntax_bitwise_or(graph, a, b, b);
						if(!result)
						{
							return NULL;
						}
					}
					else
					{
						symbol_t *result;
						result = syntax_bitwise_xor(graph, a, b, b);
						if(!result)
						{
							return NULL;
						}
					}
				}
			}
			if (symbol_check_flag(a, SYMBOL_FLAG_RIGHT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					symbol_t *result;
					result = syntax_bitwise_xor(graph, a, b, b);
					if(!result)
					{
						return NULL;
					}
				}
			}
		}
	}	else {
		symbol_t *result;
		result = syntax_bitwise_xor(graph, root, subroot, current);
		if(!result)
		{
			return NULL;
		}
		return result;
	}
	return current;
}

static symbol_t *
syntax_logical_and(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	if (symbol_check_flag(current, SYMBOL_FLAG_LAND))
	{
		symbol_t *a;
		for (a = current->begin;(a != current->end); a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_LEFT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					symbol_t *result;
					result = syntax_bitwise_or(graph, a, b, b);
					if(!result)
					{
						return NULL;
					}
				}
			}
			if (symbol_check_flag(a, SYMBOL_FLAG_RIGHT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					symbol_t *result;
					result = syntax_bitwise_or(graph, a, b, b);
					if(!result)
					{
						return NULL;
					}
				}
			}
		}
	}
	else
	{
		symbol_t *result;
		result = syntax_bitwise_or(graph, root, subroot, current);
		if(!result)
		{
			return NULL;
		}
		return result;
	}
	return current;
}

static symbol_t *
syntax_logical_or(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	if (symbol_check_flag(current, SYMBOL_FLAG_LOR))
	{
		symbol_t *a;
		for (a = current->begin;(a != current->end); a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_LEFT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					symbol_t *result;
					result = syntax_logical_and(graph, a, b, b);
					if(!result)
					{
						return NULL;
					}
				}
			}
			if (symbol_check_flag(a, SYMBOL_FLAG_RIGHT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					symbol_t *result;
					result = syntax_logical_and(graph, a, b, b);
					if(!result)
					{
						return NULL;
					}
				}
			}
		}
	}
	else
	{
		symbol_t *result;
		result = syntax_logical_and(graph, root, subroot, current);
		if(!result)
		{
			return NULL;
		}
		return result;
	}
	return current;
}

static symbol_t *
syntax_expression(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	if (symbol_check_flag(current, SYMBOL_FLAG_CONDITIONAL))
	{
		symbol_t *a;
		for (a = current->begin;(a != current->end); a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_CONDITION))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					symbol_t *result;
					result = syntax_logical_or(graph, a, b, b);
					if(!result)
					{
						return NULL;
					}
				}
			}
			else if (symbol_check_flag(a, SYMBOL_FLAG_TRUE))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					symbol_t *result;
					result = syntax_expression(graph, a, b, b);
					if(!result)
					{
						return NULL;
					}
				}
			}
			else if (symbol_check_flag(a, SYMBOL_FLAG_FALSE))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					symbol_t *result;
					result = syntax_expression(graph, a, b, b);
					if(!result)
					{
						return NULL;
					}
				}
			}
		}
	}
	else
	{
		symbol_t *result;
		result = syntax_logical_or(graph, root, subroot, current);
		if(!result)
		{
			return NULL;
		}
		return result;
	}
	return current;
}

static symbol_t *
syntax_assign(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	if (symbol_check_flag(current, SYMBOL_FLAG_ASSIGN))
	{
		symbol_t *a;
		for (a = current->begin;(a != current->end); a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_LEFT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					symbol_t *result;
					result = syntax_expression(graph, a, b, b);
					if(!result)
					{
						return NULL;
					}
				}
			}
		
			if (symbol_check_flag(a, SYMBOL_FLAG_RIGHT))
			{
				symbol_t *b;
				for (b = a->begin;(b != a->end); b = b->next)
				{
					symbol_t *result;
					result = syntax_expression(graph, a, b, b);
					if(!result)
					{
						return NULL;
					}
				}
			}
		}
	}
	else if (!symbol_check_flag(current, SYMBOL_FLAG_DEFINE))
	{
		symbol_t *result;
		result = syntax_expression(graph, root, subroot, current);
		if(!result)
		{
			return NULL;
		}
		return result;
	}
	return current;
}

static symbol_t *
syntax_return(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		symbol_t *result;
		result = syntax_expression(graph, current, a, a);
		if(!result)
		{
			return NULL;
		}
	}

	return current;
}

static symbol_t *
syntax_continue(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		symbol_t *result;
		result = syntax_expression(graph, current, a, a);
		if(!result)
		{
			return NULL;
		}
	}

	return current;
}

static symbol_t *
syntax_break(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		symbol_t *result;
		result = syntax_expression(graph, current, a, a);
		if(!result)
		{
			return NULL;
		}
	}

	return current;
}

static symbol_t *
syntax_throw(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		symbol_t *result;
		result = syntax_expression(graph, current, a, a);
		if(!result)
		{
			return NULL;
		}
	}

	return current;
}

static symbol_t *
syntax_var(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			int32_t result;
			result = syntax_duplicated_in_set(graph, root, subroot, a);
			if(!result)
			{
				return NULL;
			}
		}
	}

	return current;
}

static symbol_t *
syntax_if(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			int32_t result;
			result = syntax_duplicated_in_set(graph, root, subroot, a);
			if (!result)
			{
				return NULL;
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_CONDITION))
		{
			symbol_t *result;
			result = syntax_block(graph, current, a, a);
			if(!result)
			{
				return NULL;
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_BLOCK))
		{
			symbol_t *result;
			result = syntax_block(graph, current, a, a);
			if(!result)
			{
				return NULL;
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_ELSE))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				if (symbol_check_flag(b, SYMBOL_FLAG_IF))
				{
					symbol_t *result;
					result = syntax_if(graph, a, b, b);
					if(!result)
					{
						return NULL;
					}
				}
				else
				{
					symbol_t *result;
					result = syntax_block(graph, a, b, b);
					if(!result)
					{
						return NULL;
					}
				}
			}
		}
	}

	return current;
}

static symbol_t *
syntax_parameter(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			int32_t result;
			result = syntax_duplicated_in_set(graph, root, subroot, a);
			if(!result)
			{
				return NULL;
			}
		}
	}

	return current;
}

static symbol_t *
syntax_catch(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_BLOCK))
		{
			symbol_t *result;
			result = syntax_block(graph, current, a, a);
			if(!result)
			{
				return NULL;
			}

		}
	}

	return current;
}

static symbol_t *
syntax_try(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_CATCH))
		{
			symbol_t *result;
			result = syntax_catch(graph, current, a, a);
			if(!result)
			{
				return NULL;
			}

		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_BLOCK))
		{
			symbol_t *result;
			result = syntax_block(graph, current, a, a);
			if(!result)
			{
				return NULL;
			}

		}
	}

	return current;
}

static symbol_t *
syntax_for_init(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_VAR))
		{
			symbol_t *result;
			result = syntax_var(graph, current, a, a);
			if(!result)
			{
				return NULL;
			}

		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_ASSIGN))
		{
			symbol_t *result;
			result = syntax_assign(graph, current, a, a);
			if(!result)
			{
				return NULL;
			}

		}
	}

	return current;
}

static symbol_t *
syntax_for_step(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_ASSIGN))
		{
			symbol_t *result;
			result = syntax_assign(graph, current, a, a);
			if(!result)
			{
				return NULL;
			}
		}
	}

	return current;
}

static symbol_t *
syntax_for(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			int32_t result;
			result = syntax_duplicated_in_set(graph, root, subroot, a);
			if (!result)
			{
				return 0;
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_INITIALIZER))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				if (symbol_check_flag(a, SYMBOL_FLAG_FOR_INIT))
				{
					symbol_t *result;
					result = syntax_for_init(graph, a, b, b);
					if(!result)
					{
						return NULL;
					}
				}
				else if (symbol_check_flag(a, SYMBOL_FLAG_VAR))
				{
					symbol_t *result;
					result = syntax_var(graph, a, b, b);
					if(!result)
					{
						return NULL;
					}
				}
				else if (symbol_check_flag(a, SYMBOL_FLAG_ASSIGN))
				{
					symbol_t *result;
					result = syntax_assign(graph, a, b, b);
					if(!result)
					{
						return NULL;
					}
				}
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_INCREMENTOR))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				if (symbol_check_flag(a, SYMBOL_FLAG_FOR_STEP))
				{
					symbol_t *result;
					result = syntax_for_step(graph, a, b, b);
					if(!result)
					{
						return NULL;
					}
				}
				else if (symbol_check_flag(a, SYMBOL_FLAG_ASSIGN))
				{
					symbol_t *result;
					result = syntax_assign(graph, a, b, b);
					if(!result)
					{
						return NULL;
					}
				}
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_BLOCK))
		{
			symbol_t *result;
			result = syntax_block(graph, current, a, a);
			if(!result)
			{
				return NULL;
			}
		}
	}

	return current;
}

static symbol_t *
syntax_forin(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end); a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			int32_t result;
			result = syntax_duplicated_in_set(graph, root, subroot, a);
			if (!result)
			{
				return NULL;
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_INITIALIZER))
		{
			symbol_t *b;
			for (b = a->begin;(b != a->end); b = b->next)
			{
				if (symbol_check_flag(a, SYMBOL_FLAG_VAR))
				{
					symbol_t *result;
					result = syntax_var(graph, a, b, b);
					if(!result)
					{
						return NULL;
					}
				}
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_EXPRESSION))
		{
			symbol_t *result;
			result = syntax_expression(graph, current, a, a);
			if(!result)
			{
				return NULL;
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_BLOCK))
		{
			symbol_t *result;
			result = syntax_block(graph, current, a, a);
			if(!result)
			{
				return NULL;
			}
		}
	}
	return current;
}

static symbol_t *
syntax_statement(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *result = NULL;

	if (symbol_check_flag(current, SYMBOL_FLAG_BLOCK))
	{
		result = syntax_block(graph, root, subroot, current);
	}
	else if (symbol_check_flag(current, SYMBOL_FLAG_IF))
	{
		result = syntax_if(graph, root, subroot, current);
	}
	else if (symbol_check_flag(current, SYMBOL_FLAG_TRY))
	{
		result = syntax_try(graph, root, subroot, current);
	}
	else if (symbol_check_flag(current, SYMBOL_FLAG_FOR))
	{
		result = syntax_for(graph, root, subroot, current);
	}
	else if (symbol_check_flag(current, SYMBOL_FLAG_FORIN))
	{
		result = syntax_forin(graph, root, subroot, current);
	}
	else if (symbol_check_flag(current, SYMBOL_FLAG_FUNCTION))
	{
		result = syntax_function(graph, root, subroot, current);
	}
	else if (symbol_check_flag(current, SYMBOL_FLAG_CONTINUE))
	{
		result = syntax_continue(graph, root, subroot, current);
	}
	else if (symbol_check_flag(current, SYMBOL_FLAG_BREAK))
	{
		result = syntax_break(graph, root, subroot, current);
	}
	else if (symbol_check_flag(current, SYMBOL_FLAG_RETURN))
	{
		result = syntax_return(graph, root, subroot, current);
	}
	else if (symbol_check_flag(current, SYMBOL_FLAG_THROW))
	{
		result = syntax_throw(graph, root, subroot, current);
	}
	else if (symbol_check_flag(current, SYMBOL_FLAG_VAR))
	{
		result = syntax_var(graph, root, subroot, current);
	}
	else
	{
		result = syntax_assign(graph, root, subroot, current);
	}

	return result;
}

static symbol_t *
syntax_block(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;(a != current->end);a = a->next)
	{
		symbol_t *result;
		result = syntax_statement(graph, current, a, a);
		if(!result)
		{
			return NULL;
		}
	}
	return current;
}

static symbol_t *
syntax_import(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_FIELD))
		{
			int32_t result;
			result = syntax_duplicated_in_name(graph, current, a, a);
			if(!result)
			{
				return NULL;
			}
			result = syntax_duplicated_in_name(graph, root, current, a);
			if(!result)
			{
				return NULL;
			}
		}
	}
	return current;
}

static symbol_t *
syntax_type_parameter(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			int32_t result;
			result = syntax_duplicated_in_set(graph, root, a, a);
			if(!result)
			{
				return 0;
			}
		}
	}
	return current;
}

static symbol_t *
syntax_heritage(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			int32_t result;
			result = syntax_duplicated_in_set(graph, root, a, a);
			if(!result)
			{
				return NULL;
			}
			symbol_t *b;
			for(b = root->begin; b != root->end; b = b->next)
			{
				if (symbol_check_flag(b, SYMBOL_FLAG_TYPE_PARAMETER))
				{
					result = syntax_duplicated_in_name(graph, root, b, b);
					if(!result)
					{
						return NULL;
					}
				}
			}
		}
	}

	return current;
}

static symbol_t *
syntax_method(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *a;
	for (a = root->begin; a != root->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_TYPE_PARAMETER))
		{
			int32_t result;
			result = syntax_duplicated_in_name(graph, root, a, current);
			if(!result)
			{
				return NULL;
			}
		}
		
		if (symbol_check_flag(a, SYMBOL_FLAG_HERITAGE))
		{
			int32_t result;
			result = syntax_duplicated_in_name(graph, root, a, current);
			if(!result)
			{
				return NULL;
			}
		}
	}

	for (a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			int32_t result;
			result = syntax_duplicated_in_set_by_parameter(graph, root, subroot, a);
			if (!result)
			{
				return NULL;
			}
		}
		
		if (symbol_check_flag(a, SYMBOL_FLAG_BLOCK))
		{
			symbol_t *result;
			result = syntax_block(graph, current, a, a);
			if (!result)
			{
				return NULL;
			}
		}
	}

	return current;
}

static symbol_t *
syntax_enum(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	if (symbol_check_flag(root, SYMBOL_FLAG_CLASS))
	{
		symbol_t *a;
		for(a = root->begin; a != root->end; a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_TYPE_PARAMETER))
			{
				int32_t result;
				result = syntax_duplicated_in_name(graph, root, a, current);
				if(!result)
				{
					return NULL;
				}
			}
			
			if (symbol_check_flag(a, SYMBOL_FLAG_HERITAGE))
			{
				int32_t result;
				result = syntax_duplicated_in_name(graph, root, a, current);
				if(!result)
				{
					return NULL;
				}
			}
		}
	}

	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			int32_t result;
			result = syntax_duplicated_in_set(graph, root, subroot, a);
			if(!result)
			{
				return NULL;
			}
		}
	}

	return current;
}

static symbol_t *
syntax_property(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	if (symbol_check_flag(root, SYMBOL_FLAG_CLASS))
	{
		symbol_t *a;
		for(a = root->begin; a != root->end; a = a->next)
		{
			if (symbol_check_flag(a, SYMBOL_FLAG_TYPE_PARAMETER))
			{
				int32_t result;
				result = syntax_duplicated_in_name(graph, root, a, current);
				if(!result)
				{
					return NULL;
				}
			}
			
			if (symbol_check_flag(a, SYMBOL_FLAG_HERITAGE))
			{
				int32_t result;
				result = syntax_duplicated_in_name(graph, root, a, current);
				if(!result)
				{
					return NULL;
				}
			}
		}
	}

	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			int32_t result;
			result = syntax_duplicated_in_set(graph, root, subroot, a);
			if(!result)
			{
				return NULL;
			}
		}
	}

	return current;
}

static symbol_t *
syntax_class(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;a != current->end;a = a->next)
	{
		// attribute
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			int32_t result;
			result = syntax_duplicated_in_set(graph, root, subroot, a);
			if (!result)
			{
				return NULL;
			}
		}
		
		if (symbol_check_flag(a, SYMBOL_FLAG_HERITAGE))
		{
			symbol_t *result;
			result = syntax_heritage(graph, current, a, a);
			if(!result)
			{
				return NULL;
			}

		}

		if (symbol_check_flag(a, SYMBOL_FLAG_TYPE_PARAMETER))
		{
			symbol_t *result;
			result = syntax_type_parameter(graph, current, a, a);
			if(!result)
			{
				return NULL;
			}
		}
		
		// subclass
		if (symbol_check_flag(a, SYMBOL_FLAG_CLASS))
		{
			symbol_t *result;
			result = syntax_class(graph, current, a, a);
			if(!result)
			{
				return NULL;
			}
		}
		
		if (symbol_check_flag(a, SYMBOL_FLAG_METHOD))
		{
			symbol_t *result;
			result = syntax_method(graph, current, a, a);
			if(!result)
			{
				return NULL;
			}
		}
		
		if (symbol_check_flag(a, SYMBOL_FLAG_ENUM))
		{
			symbol_t *result;
			result = syntax_enum(graph, current, a, a);
			if(!result)
			{
				return NULL;
			}
		}
		
		if (symbol_check_flag(a, SYMBOL_FLAG_PROPERTY))
		{
			symbol_t *result;
			result = syntax_property(graph, current, a, a);
			if(!result)
			{
				return NULL;
			}
		}
	}

	return current;
}

static symbol_t *
syntax_type(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			int32_t result;
			result = syntax_duplicated_in_set(graph, root, subroot, a);
			if(!result)
			{
				return NULL;
			}
		}
	}
	return current;
}

static symbol_t *
syntax_field(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			int32_t result;
			result = syntax_duplicated_in_set(graph, root, a, a);
			if(!result)
			{
				return 0;
			}
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_TYPE))
		{
			symbol_t *b;
			for(b = a->begin; b != a->end; b = b->next)
			{
				symbol_t *result;
				result = syntax_find_connectable(a, b);
				if (!result)
				{
					syntax_error(graph, a, "type not found\n");
					return NULL;
				}
			}
		}
	}
	return current;
}

static symbol_t *
syntax_function(graph_t *graph, symbol_t *root, symbol_t *subroot, symbol_t *current)
{
	symbol_t *a;
	for (a = current->begin;a != current->end;a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_NAME))
		{
			int32_t result;
			result = syntax_duplicated_in_set(graph, root, subroot, a);
			if (!result)
			{
				return NULL;
			}
			symbol_t *b;
			for(b = current->begin; b != current->end; b = b->next)
			{
				if (symbol_check_flag(b, SYMBOL_FLAG_FIELD))
				{
					result = syntax_duplicated_in_name(graph, current, b, b);
					if(!result)
					{
						return NULL;
					}
				}

				if (symbol_check_flag(b, SYMBOL_FLAG_TYPE_PARAMETER))
				{
					result = syntax_duplicated_in_name(graph, current, b, b);
					if(!result)
					{
						return NULL;
					}
				}
			}
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_FIELD))
		{
			symbol_t *result;
			result = syntax_field(graph, current, a, a);
			if(!result)
			{
				return NULL;
			}
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_TYPE_PARAMETER))
		{
			symbol_t *result;
			result = syntax_type_parameter(graph, current, a, a);
			if(!result)
			{
				return NULL;
			}
		}

		if (symbol_check_flag(a, SYMBOL_FLAG_BLOCK))
		{
			symbol_t *result;
			result = syntax_block(graph, current, a, a);
			if(!result)
			{
				return NULL;
			}
		}
	}

	return current;
}



static symbol_t *
syntax_module(graph_t *graph, symbol_t *current)
{
	int32_t found = 0;
	symbol_t *a;
	for(a = current->begin; a != current->end; a = a->next)
	{
		/*
		if (symbol_check_flag(a, SYMBOL_FLAG_IMPORT))
		{
			symbol_t *result;
			result = syntax_import(graph, current, a, a);
			if(!result)
			{
				return NULL;
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_CLASS))
		{
			symbol_t *result;
			result = syntax_class(graph, current, a, a);
			if(!result)
			{
				return NULL;
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_ENUM))
		{
			symbol_t *result;
			result = syntax_enum(graph, current, a, a);
			if(!result)
			{
				return NULL;
			}
		}
		else if (symbol_check_flag(a, SYMBOL_FLAG_VAR))
		{
			symbol_t *result;
			result = syntax_var(graph, current, a, a);
			if(!result)
			{
				return NULL;
			}
		}
		else 
		*/
		if (symbol_check_flag(a, SYMBOL_FLAG_FUNCTION))
		{
			symbol_t *b;
			for (b = a->begin;b != a->end;b = b->next)
			{
				if (symbol_check_flag(b, SYMBOL_FLAG_NAME))
				{
					symbol_t *c;
					for (c = b->begin;c != b->end;c = c->next)
					{
						if (syntax_subset_compare_string(c, "init"))
						{
							found += 1;
						}
					}
				}

				if (symbol_check_flag(b, SYMBOL_FLAG_FIELD))
				{
					found = 0;
				}
			}

			if (found < 1)
			{
				found = 0;
				//continue;
			}

			symbol_t *result;
			result = syntax_function(graph, current, a, a);
			if(!result)
			{
				return NULL;
			}
		}
	}

	if (found < 1)
	{
		//syntax_error(graph, current, "missing init\n");
		//return NULL;
	}

	return current;
}

int32_t
syntax_run(graph_t *graph)
{
	symbol_t *symbol;
	symbol = (symbol_t *)graph->symbol;

	symbol_t *a;
	for(a = symbol->begin; a != symbol->end; a = a->next)
	{
		if (symbol_check_flag(a, SYMBOL_FLAG_MODULE))
		{
			symbol_t *result;
			result = syntax_module(graph, a);
			if(!result)
			{
				return 0;
			}
		}
	}

	return 1;
}


